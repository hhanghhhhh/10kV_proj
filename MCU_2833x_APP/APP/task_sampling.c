#include "task_sampling.h"
#include "app_context.h"
#include "drv_digipot.h"
#include "drv_ModbusData.h"
#include "task_scope.h"

#define SAMPLING_TASK_NPLC_SAMPLE_COUNT (2000.0F)

static volatile Uint16 sampling_source = SAMPLING_SOURCE_MODBUS;
static Uint32 scope_div_factor = 1UL;

static float32 LimitParameter(float32 value, float32 minimum, float32 maximum);
static void SamplingTask_ApplyCurrentRange(float32 i_range);
static void SamplingTask_ProcessParameters(void);
static void SamplingTask_ProcessTrigger(void);
static void SamplingTask_ProcessResult(void);

/* 初始化已应用参数快照，并配置McBSP-B数字电位计通道。 */
void SamplingTask_Init(void)
{
    // Digipot_Init();
    DSO_Init();

    scope_div_factor =
        (app_context.ad7982.target_count + DSO_BUF_LEN - 1UL) /
        DSO_BUF_LEN;
}

/* 记录本次触发来源，最新请求直接重新开始计数。 */
void SamplingTask_StartAverage(Uint16 source)
{
    sampling_source = source;
    app_context.ad7982.calc_done = 0U;
    DSO_CaptureStart(scope_div_factor);
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
static float32 LimitParameter(float32 value, float32 minimum, float32 maximum)
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

/* 暂留电流挡位GPIO配置入口，后续补充具体电平组合。 */
static void SamplingTask_ApplyCurrentRange(float32 i_range)
{
    (void)i_range;
}

/* 应用Modbus采样参数，电位计和挡位仅在参数变化时更新。 */
static void SamplingTask_ProcessParameters(void)
{
    float32 i_range;
    float32 tpl0501_value;
    float32 ad5290_value;
    float32 nplc;
    static float32 applied_i_range = -0.1F;
    static float32 applied_tpl0501_value = -0.1F;
    static float32 applied_ad5290_value = -0.1F;

    // 上位机参数最大最小值限制
    i_range = LimitParameter(mgmd_stSCIRx.i_range, 0.0F, 1.0F);
    tpl0501_value = LimitParameter(mgmd_stSCIRx.tpl0501_value, 0.0F, 1.0F);
    ad5290_value = LimitParameter(mgmd_stSCIRx.ad5290_value, 0.0F, 1.0F);
    nplc = LimitParameter(mgmd_stSCIRx.nplc, 0.01F, 1.0F);

    app_context.ad7982.target_count =
        (Uint32)(nplc * SAMPLING_TASK_NPLC_SAMPLE_COUNT + 0.5F);
    scope_div_factor =
        (app_context.ad7982.target_count + DSO_BUF_LEN - 1UL) /
        DSO_BUF_LEN;

    if (i_range != applied_i_range)
    {
        SamplingTask_ApplyCurrentRange(i_range);
        applied_i_range = i_range;
    }

    if (tpl0501_value != applied_tpl0501_value)
    {
        (void)Digipot_WriteTpl0501((Uint16)(tpl0501_value + 0.5F));
        applied_tpl0501_value = tpl0501_value;
    }

    if (ad5290_value != applied_ad5290_value)
    {
        (void)Digipot_WriteAd5290((Uint16)(ad5290_value + 0.5F));
        applied_ad5290_value = ad5290_value;
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
    float32 average_value;

    mgmd_stSCIRx.isamp_live = (float32)app_context.ad7982.live_adc_value;

    if (app_context.ad7982.calc_done == 0U)
    {
        return;
    }

    app_context.ad7982.calc_done = 0U;
    average_value = app_context.ad7982.done_sum /
                    (float32)app_context.ad7982.done_count;
    app_context.ad7982.final_average = average_value;
    mgmd_stSCIRx.isamp_avg = average_value;

    if (sampling_source == SAMPLING_SOURCE_CAN)
    {
        (void)CanSample_SendCompleted(&app_context.can_sample,
                                      CAN_SAMPLE_DONE_OK);
    }
}
