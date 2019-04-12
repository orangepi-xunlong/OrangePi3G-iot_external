#ifdef BUILD_LK
    #include <platform/mt_gpio.h>
#else
    #include <linux/string.h>
    #include <mach/mt_gpio.h>
    #include <mach/upmu_common.h>
#endif
#include "lcm_drv.h"

#if (defined(BUILD_UBOOT) || defined(BUILD_LK))
#define LCM_DEBUG(fmt,arg...)  printf("[otm1282][uboot]""[%s]"fmt"\n",__func__,##arg)
#else
#define LCM_DEBUG(fmt,arg...)  printk("[otm1282][kernel]""[%s]"fmt"\n",__func__,##arg)
#endif

#define LCM_ID 0x1282

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH         (240)
#define FRAME_HEIGHT        (432)

#define REGFLAG_DELAY             							0xAB
#define REGFLAG_END_OF_TABLE      							0xAA   // END OF REGISTERS MARKER

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++) {
        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);				

                if (cmd != 0xFF && cmd != 0x2C && cmd != 0x3C) {
                    //#if defined(BUILD_UBOOT)
                    //	printf("[DISP] - uboot - REG_R(0x%x) = 0x%x. \n", cmd, table[i].para_list[0]);
                    //#endif
                    while(read_reg(cmd) != table[i].para_list[0]);		
                }
        }
    }
}

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
    {0x29, 1, {0x00}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_in_setting[] = {
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Sleep Mode On
    {0x10, 1, {0x00}},

    {REGFLAG_DELAY, 50, {}},
    {0x4F, 1, {0x01}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));

    params->type   = LCM_TYPE_DSI;

    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

    // enable tearing-free
    params->dbi.te_mode 		= LCM_DBI_TE_MODE_VSYNC_ONLY;
    params->dbi.te_edge_polarity	= LCM_POLARITY_RISING;

    params->dsi.mode                    = CMD_MODE;
    //params->dsi.mode                    = SYNC_PULSE_VDO_MODE;

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM		= LCM_ONE_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB666;

    params->dsi.intermediat_buffer_num = 2;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage

    params->dsi.PS=LCM_PACKED_PS_18BIT_RGB666;

    params->dsi.word_count=240*3;       //DSI CMD mode need set these two bellow params, different to 6577
    params->dsi.vertical_active_line=432;

    params->dsi.vertical_sync_active                            = 2;
    params->dsi.vertical_backporch                              = 8;
    params->dsi.vertical_frontporch                             = 10;
    params->dsi.vertical_active_line                            = FRAME_HEIGHT;

    params->dsi.horizontal_sync_active                          = 10;
    params->dsi.horizontal_backporch                            = 10;
    params->dsi.horizontal_frontporch                           = 10;
    params->dsi.horizontal_active_pixel                         = FRAME_WIDTH;
    params->dsi.compatibility_for_nvk = 0;              // this parameter would be set to 1 if DriverIC is NTK's and when force match DSI clock for NTK's


    // Bit rate calculation
    params->dsi.PLL_CLOCK = 182;
}

static void init_lcm_registers(void)
{
    unsigned int data_array[16];

    data_array[0]=0x00033902;
    data_array[1]=0x00000044;
    dsi_set_cmdq(data_array, 2, 1);

    data_array[0]=0x00351500;
    dsi_set_cmdq(data_array, 1, 1);

    data_array[0] = 0x00110500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(180);

    data_array[0]= 0x00290500;
    dsi_set_cmdq(data_array, 1, 1);
    MDELAY(20);
}

static void lcm_init(void)
{
    upmu_set_rg_vgp1_vosel(6);
    upmu_set_rg_vgp1_en(1);
    mt_set_gpio_out(GPIO141, GPIO_OUT_ONE);

    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(200);

    init_lcm_registers();
}

static void lcm_suspend(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(200);
    
    upmu_set_rg_vgp1_en(0);
    mt_set_gpio_out(GPIO141, GPIO_OUT_ZERO);
}

static void lcm_resume(void)
{
    lcm_init();
}

static void lcm_update(unsigned int x, unsigned int y,
        unsigned int width, unsigned int height)
{
    unsigned int x0 = x;
    unsigned int y0 = y;
    unsigned int x1 = x0 + width - 1;
    unsigned int y1 = y0 + height - 1;

    unsigned char x0_MSB = ((x0>>8)&0xFF);
    unsigned char x0_LSB = (x0&0xFF);
    unsigned char x1_MSB = ((x1>>8)&0xFF);
    unsigned char x1_LSB = (x1&0xFF);
    unsigned char y0_MSB = ((y0>>8)&0xFF);
    unsigned char y0_LSB = (y0&0xFF);
    unsigned char y1_MSB = ((y1>>8)&0xFF);
    unsigned char y1_LSB = (y1&0xFF);

    unsigned int data_array[16];

    data_array[0]= 0x00053902;
    data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
    data_array[2]= (x1_LSB);
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]= 0x00053902;
    data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
    data_array[2]= (y1_LSB);
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]= 0x002c3909;
    dsi_set_cmdq(data_array, 1, 0);
}

LCM_DRIVER r63302_dsi_lcm_drv = 
{
    .name	    = "r63302_dsi",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    .update         = lcm_update,
};
