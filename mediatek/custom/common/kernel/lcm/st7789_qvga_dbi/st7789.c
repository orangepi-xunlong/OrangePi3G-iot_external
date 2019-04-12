#ifdef BUILD_LK                                                                                           
#else                                                                                                     
    #include <linux/string.h>                                                                             
#endif                                                                                                    
#include "lcm_drv.h"                                                                                      
                                                                                                          
// ---------------------------------------------------------------------------                            
//  Local Constants                                                                                       
// ---------------------------------------------------------------------------                            
                                                                                                          
#define FRAME_WIDTH                  (240)
#define FRAME_HEIGHT                 (320)
#define LCM_ID       (0x8552)                                                                             
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
                                                                                                          
static __inline void send_ctrl_cmd(unsigned int cmd)                                                      
{                                                                                                         
   lcm_util.send_cmd(cmd);                                                                                
}                                                                                                         
                                                                                                          
                                                                                                          
static __inline void send_data_cmd(unsigned int data)                                                     
{                                                                                                         
   lcm_util.send_data(data&0xff);                                                                         
}                                                                                                         
                                                                                                          
                                                                                                          
static __inline unsigned int read_data_cmd(void)                                                          
{                                                                                                         
   return 0xFF&lcm_util.read_data();                                                                      
}                                                                                                         
                                                                                                          
                                                                                                          
static __inline void set_lcm_register(unsigned int regIndex,                                              
                                                            unsigned int regData)                         
{                                                                                                         
   send_ctrl_cmd(regIndex);                                                                               
   send_data_cmd(regData);                                                                                
}                                                                                                         
                                                                                                          
                                                                                                          
static void sw_clear_panel(unsigned int color)                                                            
{                                                                                                         
   unsigned short x0, y0, x1, y1, x, y;                                                                   
   unsigned short h_X_start,l_X_start,h_X_end,l_X_end,h_Y_start,l_Y_start,h_Y_end,l_Y_end;                
                                                                                                          
   x0 = (unsigned short)0;                                                                                
   y0 = (unsigned short)0;                                                                                
   x1 = (unsigned short)FRAME_WIDTH-1;                                                                    
   y1 = (unsigned short)FRAME_HEIGHT-1;                                                                   
                                                                                                          
   h_X_start=((x0&0xFF00)>>8);                                                                            
   l_X_start=(x0&0x00FF);                                                                                 
   h_X_end=((x1&0xFF00)>>8);                                                                              
   l_X_end=(x1&0x00FF);                                                                                   
                                                                                                          
   h_Y_start=((y0&0xFF00)>>8);                                                                            
   l_Y_start=(y0&0x00FF);                                                                                 
   h_Y_end=((y1&0xFF00)>>8);                                                                              
   l_Y_end=(y1&0x00FF);                                                                                   
                                                                                                          
   send_ctrl_cmd(0x2A);                                                                                   
   send_data_cmd(h_X_start);                                                                              
   send_data_cmd(l_X_start);                                                                              
   send_data_cmd(h_X_end);                                                                                
   send_data_cmd(l_X_end);                                                                                
                                                                                                          
   send_ctrl_cmd(0x2B);                                                                                   
   send_data_cmd(h_Y_start);                                                                              
   send_data_cmd(l_Y_start);                                                                              
   send_data_cmd(h_Y_end);                                                                                
   send_data_cmd(l_Y_end);                                                                                
                                                                                                          
   send_ctrl_cmd(0x29);                                                                                   
   send_ctrl_cmd(0x2C);                                                                                   
                                                                                                          
   for (y = y0; y <= y1; ++ y) {                                                                          
      for (x = x0; x <= x1; ++ x) {                                                                       
         send_data_cmd(color);                                                                            
          send_data_cmd(color);                                                                           
          send_data_cmd(color);                                                                           
      }                                                                                                   
   }                                                                                                      
}                                                                                                         
                                                                                                          
                                                                                                          
static void init_lcm_registers(void)                                                                      
 {   
								#if 1                
								send_ctrl_cmd(0x11);
								MDELAY(120);  //delay1ms 120ms
								//--------------------------------------Display Setting------------------------------------------//
								send_ctrl_cmd(0x36);
								send_data_cmd(0x00);
								
								send_ctrl_cmd(0x3a);
								send_data_cmd(0x06);
								//--------------------------------ST7789V Frame rate setting----------------------------------//
								send_ctrl_cmd(0xb2);
								send_data_cmd(0x0c);
								send_data_cmd(0x0c);
								send_data_cmd(0x00);
								send_data_cmd(0x33);
								send_data_cmd(0x33);
								
								send_ctrl_cmd(0xb7);
								send_data_cmd(0x44);  //37  
								//---------------------------------ST7789V Power setting--------------------------------------//
								send_ctrl_cmd(0xbb);
								send_data_cmd(0x37);   //2B	 35
								
								send_ctrl_cmd(0xc0);
								send_data_cmd(0x2C);	//2D
								
								send_ctrl_cmd(0xc2);
								send_data_cmd(0x01);
								
								
								send_ctrl_cmd(0xc3);
								send_data_cmd(0x11);
								
								send_ctrl_cmd(0xc4);
								send_data_cmd(0x20);   //20
								
								send_ctrl_cmd(0xc6);
								send_data_cmd(0x0a);  //  0f
								
								send_ctrl_cmd(0xd0);
								send_data_cmd(0xa4);
								send_data_cmd(0xa1);
								//--------------------------------ST7789V gamma setting---------------------------------------//
								send_ctrl_cmd(0xe0);
								send_data_cmd(0xd0);
								send_data_cmd(0x00);
								send_data_cmd(0x05);
								send_data_cmd(0x0e);
								send_data_cmd(0x15);
								send_data_cmd(0x0d);
								send_data_cmd(0x37);
								send_data_cmd(0x43);
								send_data_cmd(0x47);
								send_data_cmd(0x09);
								send_data_cmd(0x15);
								send_data_cmd(0x12);
								send_data_cmd(0x16);
								send_data_cmd(0x19);
								
								send_ctrl_cmd(0xe1);
								send_data_cmd(0xd0);
								send_data_cmd(0x00);
								send_data_cmd(0x05);
								send_data_cmd(0x0d);
								send_data_cmd(0x0c);
								send_data_cmd(0x06);
								send_data_cmd(0x2d);
								send_data_cmd(0x44);
								send_data_cmd(0x40);
								send_data_cmd(0x0e);
								send_data_cmd(0x1c);
								send_data_cmd(0x18);
								send_data_cmd(0x16);
								send_data_cmd(0x19);
								
								
								send_ctrl_cmd(0x29);
								
								
								
								#else
								                                               
								send_ctrl_cmd(0x11);                                                                      
								MDELAY(120);                                                                              
								                                                                          
								send_ctrl_cmd(0x36);                                                                      
								send_data_cmd(0x00);//40                                                                  
								                                                                          
								send_ctrl_cmd(0x3a);                                                                      
								send_data_cmd(0x06);//06                                                                  
								                                                                          
								send_ctrl_cmd(0xb2);                                                                      
								send_data_cmd(0x28);                                                                      
								send_data_cmd(0x28);                                                                      
								send_data_cmd(0x05);                                                                      
								send_data_cmd(0x33);                                                                      
								send_data_cmd(0x33);                                                                      
								                                                                          
								                                                                          
								                                                                          
								send_ctrl_cmd(0xb7);                                                                      
								send_data_cmd(0x35);                                                                      
								                                                                          
								send_ctrl_cmd(0xbb);                                                                      
								send_data_cmd(0x3c);//23                                                                  
								                                                                          
								send_ctrl_cmd(0xb1);                                                                      
								send_data_cmd(0x80);                                                                      
								send_data_cmd(0x10);                                                                      
								                                                                          
								                                                                          
								send_ctrl_cmd(0xc0);                                                                      
								send_data_cmd(0x2c);                                                                      
								                                                                          
								send_ctrl_cmd(0xc2);                                                                      
								send_data_cmd(0x01);                                                                      
								                                                                          
								send_ctrl_cmd(0xc3);                                                                      
								send_data_cmd(0x05);//14                                                                  
								                                                                          
								send_ctrl_cmd(0xc4);                                                                      
								send_data_cmd(0x20);                                                                      
								                                                                          
								send_ctrl_cmd(0xc6);                                                                      
								send_data_cmd(0x14); // 14                                                                
								                                                                          
								send_ctrl_cmd(0xd0);                                                                      
								send_data_cmd(0xa4);                                                                      
								send_data_cmd(0xa1);                                                                      
								                                                                          
								send_ctrl_cmd(0xe0);                                                                      
								send_data_cmd(0xd0);                                                                      
								send_data_cmd(0x00);                                                                      
								send_data_cmd(0x02);                                                                      
								send_data_cmd(0x07);                                                                      
								send_data_cmd(0x07);                                                                      
								send_data_cmd(0x19);                                                                      
								send_data_cmd(0x2e);                                                                      
								send_data_cmd(0x54);                                                                      
								send_data_cmd(0x41);                                                                      
								send_data_cmd(0x2d);                                                                      
								send_data_cmd(0x17);                                                                      
								send_data_cmd(0x18);                                                                      
								send_data_cmd(0x14);                                                                      
								send_data_cmd(0x18);                                                                      
								                                                                          
								send_ctrl_cmd(0xe1);                                                                      
								send_data_cmd(0xd0);                                                                      
								send_data_cmd(0x00);                                                                      
								send_data_cmd(0x02);                                                                      
								send_data_cmd(0x07);                                                                      
								send_data_cmd(0x04);                                                                      
								send_data_cmd(0x24);                                                                      
								send_data_cmd(0x2c);                                                                      
								send_data_cmd(0x44);                                                                      
								send_data_cmd(0x42);                                                                      
								send_data_cmd(0x1c);                                                                      
								send_data_cmd(0x1a);                                                                      
								send_data_cmd(0x17);                                                                      
								send_data_cmd(0x15);                                                                      
								send_data_cmd(0x18);                                                                              
								send_ctrl_cmd(0x35);                                                                      
								send_data_cmd(0x00);//40                                                                  
								                                                                          
								send_ctrl_cmd(0x44);                                                                      
								send_data_cmd(0x19);                                                                      
								                                                                          
								send_ctrl_cmd(0x29);                                                                      
								#endif                                                                                                       
                sw_clear_panel(0x0);                                                                      
                                                                                                          
}                                                                                                 
                                                                                                          
                                                                                                          
// ---------------------------------------------------------------------------                            
//  LCM Driver Implementations                                                                            
// ---------------------------------------------------------------------------                            
                                                                                                          
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)                                                
{                                                                                                         
   memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));                                                       
}                                                                                                         
                                                                                                          
                                                                                                          
static void lcm_get_params(LCM_PARAMS *params)                                                            
{                                                                                                         
        memset(params, 0, sizeof(LCM_PARAMS));                                                            
                                                                                                          
        params->type = LCM_TYPE_DBI;                                                                      
        params->ctrl = LCM_CTRL_PARALLEL_DBI;                                                             
        params->width = FRAME_WIDTH;                                                                      
        params->height = FRAME_HEIGHT;                                                                    
                                                                                                          
        //params->physical_width = PHYSICAL_WIDTH;                                                        
        //params->physical_height = PHYSICAL_HEIGHT;                                                      
        params->io_select_mode= 1;                                                                        
                                                                                                          
        params->dbi.port = 0;                                                                             
        params->dbi.clock_freq= LCM_DBI_CLOCK_FREQ_26M;                                                   
        params->dbi.data_width = LCM_DBI_DATA_WIDTH_8BITS;                                                
        params->dbi.data_format.color_order = LCM_COLOR_ORDER_RGB;                                        
    		params->dbi.data_format.trans_seq   = LCM_DBI_TRANS_SEQ_MSB_FIRST;                                    
   			 params->dbi.data_format.padding     = LCM_DBI_PADDING_ON_LSB;                                         
    		params->dbi.data_format.format      = LCM_DBI_FORMAT_RGB666;                                          
    		params->dbi.data_format.width       = LCM_DBI_DATA_WIDTH_8BITS;                                       
        params->dbi.cpu_write_bits = LCM_DBI_CPU_WRITE_8_BITS;                                            
        params->dbi.io_driving_current = LCM_DRIVING_CURRENT_8MA;                                         
                                                                                                          
        params->dbi.parallel.write_setup    = 1;                                                          
        params->dbi.parallel.write_hold     = 10;                                                         
        params->dbi.parallel.write_wait     = 15;                                                         
        params->dbi.parallel.read_setup     = 1;                                                          
        params->dbi.parallel.read_hold      = 10;                                                         
        params->dbi.parallel.read_latency   = 45;                                                         
        params->dbi.parallel.wait_period    = 1;                                                          
                                                                                                          
        params->dbi.te_mode = LCM_DBI_TE_MODE_DISABLED;                                                   
        //params->dbi.te_edge_polarity = LCM_POLARITY_RISING;                                             
}                                                                                                         
                                                                                                          
                                                                                                          
static void lcm_init(void)                                                                                
{                                                                                                         
    SET_RESET_PIN(1);                                                                                     
    MDELAY(50);                                                                                           
    SET_RESET_PIN(0);                                                                                     
    MDELAY(100);                                                                                          
    SET_RESET_PIN(1);                                                                                     
    MDELAY(100);                                                                                          
                                                                                                          
    init_lcm_registers();                                                                                 
}                                                                                                         
                                                                                                          
                                                                                                          
static void lcm_suspend(void)                                                                             
{                                                                                                         
    send_ctrl_cmd(0x28);                                                                                  
    MDELAY(100);                                                                                          
    send_ctrl_cmd(0x010);                                                                                 
}                                                                                                         
                                                                                                          
                                                                                                          
static void lcm_resume(void)                                                                              
{                                                                                                         
    send_ctrl_cmd(0x11);                                                                                  
    MDELAY(150);                                                                                          
    send_ctrl_cmd(0x029);                                                                                 
}                                                                                                         
                                                                                                          
                                                                                                          
static void lcm_update(unsigned int x, unsigned int y,                                                    
                                       unsigned int width, unsigned int height)                           
{                                                                                                         
   unsigned int h_X_start,l_X_start,h_X_end,l_X_end,h_Y_start,l_Y_start,h_Y_end,l_Y_end;                  
                                                                                                          
   unsigned int x0 = x;                                                                                   
   unsigned int y0 = y;                                                                                   
   unsigned int x1 = x0 + width - 1;                                                                      
   unsigned int y1 = y0 + height - 1;                                                                     
                                                                                                          
   h_X_start=((x0&0xFF00)>>8);                                                                            
   l_X_start=(x0&0x00FF);                                                                                 
   h_X_end=((x1&0xFF00)>>8);                                                                              
   l_X_end=(x1&0x00FF);                                                                                   
                                                                                                          
   h_Y_start=((y0&0xFF00)>>8);                                                                            
   l_Y_start=(y0&0x00FF);                                                                                 
   h_Y_end=((y1&0xFF00)>>8);                                                                              
   l_Y_end=(y1&0x00FF);                                                                                   
                                                                                                          
   send_ctrl_cmd(0x2A);                                                                                   
   send_data_cmd(h_X_start);                                                                              
   send_data_cmd(l_X_start);                                                                              
   send_data_cmd(h_X_end);                                                                                
   send_data_cmd(l_X_end);                                                                                
                                                                                                          
   send_ctrl_cmd(0x2B);                                                                                   
   send_data_cmd(h_Y_start);                                                                              
   send_data_cmd(l_Y_start);                                                                              
   send_data_cmd(h_Y_end);                                                                                
   send_data_cmd(l_Y_end);                                                                                
                                                                                                          
   send_ctrl_cmd(0x29);                                                                                   
                                                                                                          
   send_ctrl_cmd(0x2C);                                                                                   
}                                                                                                         
                                                                                                          
static unsigned int lcm_compare_id(void)                                                                  
{                                                                                                         
        unsigned  char id_tmp[4] ;                                                                        
        int lcd_id;            
        return 1;                                                                           
        send_ctrl_cmd(0x04);                                                                              
        id_tmp[0]  =  read_data_cmd();                                                                    
    id_tmp[1]  =  read_data_cmd();                                                                        
    id_tmp[2]  =  read_data_cmd();                                                                        
    id_tmp[3]  =  read_data_cmd();                                                                        
    lcd_id = ((id_tmp[2]&0xff)<<8)|(id_tmp[3]&0xff);                                                      
                                                                                                          
#ifdef BUILD_LK                                                                                           
        printf("ST7789S lcm_get_params id_tmp[2] = %d,id_tmp[3]= %d\n",id_tmp[2],id_tmp[3]);              
#endif                                                                                                    
                                                                                                          
    return (LCM_ID == lcd_id)?1:0;                                                                        
}                                                                                                         
                                                                                                          
                                                                                                          
LCM_DRIVER st7789_lcm_drv =                                                                              
{                                                                                                         
   	    .name          = "st7789s",                                                             
        .set_util_funcs = lcm_set_util_funcs,                                                             
        .get_params     = lcm_get_params,                                                                 
        .init           = lcm_init,                                                                       
        .suspend        = lcm_suspend,                                                                    
        .resume         = lcm_resume,                                                                     
        .update         = lcm_update,                                                                     
        .compare_id     = lcm_compare_id                                                                  
};                                                                                                        