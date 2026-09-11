#include "task_sampling.h"
#include "app_context.h"
#include "drv_digipot.h"
#include "drv_ModbusData.h"
#include "task_scope.h"
#include "DSP2833x_Device.h"

#define SAMPLING_TASK_NPLC_SAMPLE_COUNT (2000.0F)
#define SAMPLING_TASK_ADC_REFERENCE_VOLTAGE (4.096F)
#define SAMPLING_TASK_ADC_POSITIVE_FULL_CODE (131071.0F)
#define SAMPLING_TASK_DIGIPOT_MAX_RESISTANCE_KOHM (100U)
#define SAMPLING_TASK_DIGIPOT_CODE_SCALE (256.0F)
#define SAMPLING_TASK_DIGIPOT_MAX_CODE (255U)

#define SAMPLING_TASK_5MA_50MA_MASK (0x00000001UL)   /* GPIO64 */
#define SAMPLING_TASK_100UA_1MA_MASK (0x00000002UL)  /* GPIO65 */
#define SAMPLING_TASK_1UA_10UA_MASK (0x00000004UL)   /* GPIO66 */
#define SAMPLING_TASK_TBD_MA_MASK (0x00000008UL)     /* GPIO67 */
#define SAMPLING_TASK_IGAIN_A0_MASK (0x00000010UL)   /* GPIO68 */
#define SAMPLING_TASK_IGAIN_A1_MASK (0x00000020UL)   /* GPIO69 */
#define SAMPLING_TASK_DACOMP_OUT_MASK (0x00000040UL) /* GPIO70 */
#define SAMPLING_TASK_LC_HC_MASK (0x00000080UL)      /* GPIO71 */
#define SAMPLING_TASK_RANGE_CONTROL_MASK \
    (SAMPLING_TASK_5MA_50MA_MASK | SAMPLING_TASK_100UA_1MA_MASK | \
     SAMPLING_TASK_1UA_10UA_MASK | SAMPLING_TASK_IGAIN_A0_MASK | \
     SAMPLING_TASK_IGAIN_A1_MASK)
#define SAMPLING_TASK_CURRENT_CONTROL_MASK \
    (SAMPLING_TASK_RANGE_CONTROL_MASK | SAMPLING_TASK_TBD_MA_MASK | \
     SAMPLING_TASK_DACOMP_OUT_MASK | SAMPLING_TASK_LC_HC_MASK)

typedef struct
{
    float32 full_scale_current;
    float32 adc_full_scale_voltage;
    Uint32 gpio_set_mask;
} SamplingTask_CurrentRange_t;

static const SamplingTask_CurrentRange_t sampling_task_current_range[] =
{
    {1.0E-8F, 3.3333F, SAMPLING_TASK_IGAIN_A0_MASK |
                         SAMPLING_TASK_IGAIN_A1_MASK},
    {1.0E-7F, 3.3333F, 0UL},
    {1.0E-6F, 3.92F,   SAMPLING_TASK_1UA_10UA_MASK |
                         SAMPLING_TASK_IGAIN_A0_MASK |
                         SAMPLING_TASK_IGAIN_A1_MASK},
    {1.0E-5F, 3.92F,   SAMPLING_TASK_1UA_10UA_MASK},
    {1.0E-4F, 3.66F,   SAMPLING_TASK_100UA_1MA_MASK |
                         SAMPLING_TASK_IGAIN_A0_MASK |
                         SAMPLING_TASK_IGAIN_A1_MASK},
    {1.0E-3F, 3.66F,   SAMPLING_TASK_100UA_1MA_MASK},
    {5.0E-3F, 3.1125F, SAMPLING_TASK_5MA_50MA_MASK |
                         SAMPLING_TASK_IGAIN_A0_MASK |
                         SAMPLING_TASK_IGAIN_A1_MASK},
    {5.0E-2F, 3.1125F, SAMPLING_TASK_5MA_50MA_MASK}
};

#define SAMPLING_TASK_CURRENT_RANGE_COUNT \
    ((Uint16)(sizeof(sampling_task_current_range) / \
              sizeof(sampling_task_current_range[0])))

static volatile Uint16 sampling_source = SAMPLING_SOURCE_MODBUS;
static Uint32 scope_div_factor = 1UL;
static Uint32 scope_store_count = 1UL;
static float32 current_range_full_scale = 1.0E-8F;
static float32 current_adc_full_scale_voltage = 3.3333F;
static float32 current_per_adc_code = 0.0F; /* A/code */

static float32 SamplingTask_LimitParameter(float32 value,
                                           float32 minimum,
                                           float32 maximum);
static Uint16 SamplingTask_ConvertResistanceToCode(Uint16 resistance_kohm);
static void SamplingTask_InitCurrentRangeGpio(void);
static void SamplingTask_UpdateScopeConfig(Uint32 target_count);
static void SamplingTask_ApplyCurrentRange(float32 i_range);
static void SamplingTask_ApplyDacompOutput(Uint16 output_value);
static float32 SamplingTask_ConvertAdcCode(float32 adc_code);
static void SamplingTask_ProcessParameters(void);
static void SamplingTask_ProcessTrigger(void);
static void SamplingTask_ProcessResult(void);

/* 初始化已应用参数快照，并配置McBSP-B数字电位计通道。 */
void SamplingTask_Init(void)
{
    Digipot_Init();
    DSO_Init();
    SamplingTask_InitCurrentRangeGpio();

    SamplingTask_UpdateScopeConfig(app_context.ad7982.target_count);
}

/* 记录本次触发来源，最新请求直接重新开始计数。 */
void SamplingTask_StartAverage(Uint16 source)
{
    sampling_source = source;
    app_context.ad7982.calc_done = 0U;
    DSO_CaptureStart(scope_div_factor, scope_store_count);
    Ad7982_StartAverage(&app_context.ad7982);
}

/* 主循环中调用的采样业务处理入口。 */
void SamplingTask_Run(void)
{
    SamplingTask_ProcessParameters();
    SamplingTask_ProcessTrigger();
    SamplingTask_ProcessResult();
}

/* 将上位机参数限制在当前工程允许范围内。 */
static float32 SamplingTask_LimitParameter(float32 value,
                                           float32 minimum,
                                           float32 maximum)
{
    if (value < minimum)
    {
        return minimum;
    }
    if (value > maximum)
    {
        return maximum;
    }
    return value;
}

/* 将上位机下发的kΩ电阻值换算为8位电位计码值。 */
static Uint16 SamplingTask_ConvertResistanceToCode(Uint16 resistance_kohm)
{
    Uint16 code;

    code = (Uint16)(resistance_kohm * SAMPLING_TASK_DIGIPOT_CODE_SCALE /
                    SAMPLING_TASK_DIGIPOT_MAX_RESISTANCE_KOHM + 0.5F);
    if (code > SAMPLING_TASK_DIGIPOT_MAX_CODE)
    {
        code = SAMPLING_TASK_DIGIPOT_MAX_CODE;
    }
    return code;
}

/* GPIO64~71与XINTF复用，本工程作为电流通道控制输出使用。 */
static void SamplingTask_InitCurrentRangeGpio(void)
{
    GpioDataRegs.GPCCLEAR.all = SAMPLING_TASK_CURRENT_CONTROL_MASK;

    EALLOW;
    GpioCtrlRegs.GPCMUX1.bit.GPIO64 = 0U;
    GpioCtrlRegs.GPCMUX1.bit.GPIO65 = 0U;
    GpioCtrlRegs.GPCMUX1.bit.GPIO66 = 0U;
    GpioCtrlRegs.GPCMUX1.bit.GPIO67 = 0U;
    GpioCtrlRegs.GPCMUX1.bit.GPIO68 = 0U;
    GpioCtrlRegs.GPCMUX1.bit.GPIO69 = 0U;
    GpioCtrlRegs.GPCMUX1.bit.GPIO70 = 0U;
    GpioCtrlRegs.GPCMUX1.bit.GPIO71 = 0U;

    GpioCtrlRegs.GPCDIR.bit.GPIO64 = 1U;
    GpioCtrlRegs.GPCDIR.bit.GPIO65 = 1U;
    GpioCtrlRegs.GPCDIR.bit.GPIO66 = 1U;
    GpioCtrlRegs.GPCDIR.bit.GPIO67 = 1U;
    GpioCtrlRegs.GPCDIR.bit.GPIO68 = 1U;
    GpioCtrlRegs.GPCDIR.bit.GPIO69 = 1U;
    GpioCtrlRegs.GPCDIR.bit.GPIO70 = 1U;
    GpioCtrlRegs.GPCDIR.bit.GPIO71 = 1U;

    GpioCtrlRegs.GPCPUD.bit.GPIO64 = 1U;
    GpioCtrlRegs.GPCPUD.bit.GPIO65 = 1U;
    GpioCtrlRegs.GPCPUD.bit.GPIO66 = 1U;
    GpioCtrlRegs.GPCPUD.bit.GPIO67 = 1U;
    GpioCtrlRegs.GPCPUD.bit.GPIO68 = 1U;
    GpioCtrlRegs.GPCPUD.bit.GPIO69 = 1U;
    GpioCtrlRegs.GPCPUD.bit.GPIO70 = 1U;
    GpioCtrlRegs.GPCPUD.bit.GPIO71 = 1U;
    EDIS;
}

/* 根据平均采样点数计算波形分频和实际存储点数。 */
static void SamplingTask_UpdateScopeConfig(Uint32 target_count)
{
    scope_div_factor = (target_count + DSO_BUF_LEN - 1UL) / DSO_BUF_LEN;
    scope_store_count = target_count / scope_div_factor;
}

/* 选择能覆盖目标电流的最小挡位，并更新AD码换算比例。 */
static void SamplingTask_ApplyCurrentRange(float32 i_range)
{
    Uint16 range_index;
    float32 range_current;
    const SamplingTask_CurrentRange_t *range;

    range_current = i_range;
    if (range_current < 0.0F)
    {
        range_current = -range_current;
    }

    range_index = 0U;
    while ((range_index < (SAMPLING_TASK_CURRENT_RANGE_COUNT - 1U)) &&
           (range_current >
            sampling_task_current_range[range_index].full_scale_current))
    {
        range_index++;
    }
    range = &sampling_task_current_range[range_index];

    GpioDataRegs.GPCCLEAR.all = SAMPLING_TASK_RANGE_CONTROL_MASK;
    GpioDataRegs.GPCSET.all = range->gpio_set_mask;

    current_range_full_scale = range->full_scale_current;
    current_adc_full_scale_voltage = range->adc_full_scale_voltage;
    current_per_adc_code =
        (SAMPLING_TASK_ADC_REFERENCE_VOLTAGE * current_range_full_scale) /
        (SAMPLING_TASK_ADC_POSITIVE_FULL_CODE *
         current_adc_full_scale_voltage);
}

/* DACOMP_OUT由上位机单独控制，不参与挡位切换。 */
static void SamplingTask_ApplyDacompOutput(Uint16 output_value)
{
    if (output_value == 0U)
    {
        GpioDataRegs.GPCCLEAR.all = SAMPLING_TASK_DACOMP_OUT_MASK;
    }
    else
    {
        GpioDataRegs.GPCSET.all = SAMPLING_TASK_DACOMP_OUT_MASK;
    }
}

/* 现有18位有符号AD正满码131071对应4.096 V。 */
static float32 SamplingTask_ConvertAdcCode(float32 adc_code)
{
    return adc_code * current_per_adc_code;
}

/* 应用Modbus采样参数，电位计和挡位仅在参数变化时更新。 */
static void SamplingTask_ProcessParameters(void)
{
    float32 i_range;
    Uint16 dacomp_rc1;
    Uint16 dacomp_x;
    float32 nplc;
    Uint16 dacomp_out;
    static float32 applied_i_range = -0.1F;
    static Uint16 applied_dacomp_rc1 = 0xFFFFU;
    static Uint16 applied_dacomp_x = 0xFFFFU;
    static Uint16 applied_dacomp_out = 0xFFFFU;

    // 上位机参数最大最小值限制
    i_range = SamplingTask_LimitParameter(mgmd_stSCIRx.i_range,
                                          -5.0E-2F,
                                          5.0E-2F);
    dacomp_rc1 = mgmd_stSCIRx.dacomp_rc1;
    if (dacomp_rc1 > SAMPLING_TASK_DIGIPOT_MAX_RESISTANCE_KOHM)
    {
        dacomp_rc1 = SAMPLING_TASK_DIGIPOT_MAX_RESISTANCE_KOHM;
    }
    dacomp_x = mgmd_stSCIRx.dacomp_x;
    if (dacomp_x > SAMPLING_TASK_DIGIPOT_MAX_RESISTANCE_KOHM)
    {
        dacomp_x = SAMPLING_TASK_DIGIPOT_MAX_RESISTANCE_KOHM;
    }
    nplc = SamplingTask_LimitParameter(mgmd_stSCIRx.nplc,
                                       0.01F,
                                       1.0F);
    dacomp_out = (mgmd_stSCIRx.dacomp_out == 0U) ? 0U : 1U;
    mgmd_stSCIRx.dacomp_out = dacomp_out;

    // 计算 nplc 对应的采样点数
    app_context.ad7982.target_count = (Uint32)(nplc * SAMPLING_TASK_NPLC_SAMPLE_COUNT + 0.5F);
    SamplingTask_UpdateScopeConfig(app_context.ad7982.target_count);

    // 切换挡位
    if (i_range != applied_i_range)
    {
        SamplingTask_ApplyCurrentRange(i_range);
        applied_i_range = i_range;
    }

    if (dacomp_rc1 != applied_dacomp_rc1)
    {
        (void)Digipot_WriteTpl0501(
            SamplingTask_ConvertResistanceToCode(dacomp_rc1));
        applied_dacomp_rc1 = dacomp_rc1;
    }

    if (dacomp_x != applied_dacomp_x)
    {
        (void)Digipot_WriteAd5290(
            SamplingTask_ConvertResistanceToCode(dacomp_x));
        applied_dacomp_x = dacomp_x;
    }

    if (dacomp_out != applied_dacomp_out)
    {
        SamplingTask_ApplyDacompOutput(dacomp_out);
        applied_dacomp_out = dacomp_out;
    }
}

/* 上位机写1触发一次采样，接收后自动清零命令。 */
static void SamplingTask_ProcessTrigger(void)
{
    if (mgmd_stSCIRx.sample_trigger != 1U)
    {
        return;
    }

    mgmd_stSCIRx.sample_trigger = 0U;
    SamplingTask_StartAverage(SAMPLING_SOURCE_MODBUS);
}

/* 发布采样值，完成后仅对CAN触发请求回复报文。 */
static void SamplingTask_ProcessResult(void)
{
    float32 average_adc_code;
    float32 average_current;

    mgmd_stSCIRx.isamp_live = SamplingTask_ConvertAdcCode(
        (float32)app_context.ad7982.live_adc_value);

    if (app_context.ad7982.calc_done == 0U)
    {
        return;
    }

    app_context.ad7982.calc_done = 0U;
    average_adc_code = app_context.ad7982.done_sum /
                       (float32)app_context.ad7982.done_count;
    average_current = SamplingTask_ConvertAdcCode(average_adc_code);
    app_context.ad7982.final_average = average_current;
    mgmd_stSCIRx.isamp_avg = average_current;

    if (sampling_source == SAMPLING_SOURCE_CAN)
    {
        (void)CanSample_SendCompleted(&app_context.can_sample,
                                      CAN_SAMPLE_DONE_OK);
    }
}
