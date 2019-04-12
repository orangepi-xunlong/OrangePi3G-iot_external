

#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/irq.h>
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/kobject.h>
#include <linux/earlysuspend.h>
#include <linux/platform_device.h>
#include <asm/atomic.h>

#include "hct_devices.h"

typedef int (*hct_dev_print)(struct seq_file *se_f);

typedef int (*hct_dev_set_used)(char * module_name, int pdata);

struct hct_lcm_device_idnfo
{
    struct list_head hct_list;  // list in hct_devices_info
    char lcm_module_descrip[50];
    struct list_head lcm_list;  // list in hct_lcm_info
    LCM_DRIVER* pdata;
    campatible_type type;
};

struct hct_camera_device_dinfo
{
    struct list_head hct_list;
    char camera_module_descrip[20];    
    struct list_head camera_list;
    int  camera_id;
    ACDK_KD_SENSOR_INIT_FUNCTION_STRUCT* pdata;
    campatible_type type;
    CAMERA_DUAL_CAMERA_SENSOR_ENUM  cam_type;
};

struct hct_accsensor_device_dinfo
{
    struct list_head hct_list;
    char sensor_descrip[20];
    struct list_head sensor_list;
    int dev_addr;
    int ps_direction;   // ps is throthold, als/msensor is diriction
    campatible_type type;
    struct sensor_init_info * pdata;
};

struct hct_touchpanel_device_dinfo
{
    struct list_head hct_list;     // list in hct_devices_info
    char touch_descrip[20];
    struct list_head touch_list;
    int dev_addr;
    struct tpd_driver_t * pdata;
//    int ps_direction;   // ps is throthold, als/msensor is diriction
    campatible_type type;
};

struct hct_type_info
{
    struct list_head hct_device;  // list in hct_devices_info
    struct list_head hct_dinfo;    // list in hct_touch_device_dinfo
    hct_dev_print type_print;
    hct_dev_set_used set_used;
    int dev_type;
    void * current_used_dinfo;
    struct mutex	info_mutex;
};


struct hct_devices_info
{
         struct list_head  type_list;   // hct_type_info list
         struct list_head  dev_list;    // all_list

        struct mutex	de_mutex;
};

struct hct_devices_info * hct_devices;
static int hct_camera_info_print(struct seq_file *se_f);
static int hct_touchpanel_info_print(struct seq_file *se_f);
static int hct_accsensor_info_print(struct seq_file *se_f);


static int hct_lcm_set_used(char * module_name,int pdata);
static int hct_camera_set_used(char * module_name, int pdata);
static int hct_touchpanel_set_used(char * module_name, int pdata);
static int hct_accsensor_set_used(char * module_name, int pdata);


static int hct_lcm_info_print(struct seq_file *se_f)
{
    
    struct hct_type_info * lcm_type_info;    
    struct hct_lcm_device_idnfo * plcm_device;
    int flag = -1;

    seq_printf(se_f, "---------LCM USAGE--------\t \n");
    
    list_for_each_entry(lcm_type_info,&hct_devices->type_list,hct_device){
            if(lcm_type_info->dev_type == ID_LCM_TYPE)
           {
               flag =1;  // this mean type is ok
               break;
           }
       }
    if(flag == 1)
    {
              list_for_each_entry(plcm_device,&lcm_type_info->hct_dinfo,lcm_list){
                seq_printf(se_f, "      %s\t:   ",plcm_device->lcm_module_descrip);
                if(plcm_device->type == DEVICE_SUPPORTED)
                   seq_printf(se_f, "  supported \n");
                else
                    seq_printf(se_f, "  used \n");
                }
    }
    else
    {
        seq_printf(se_f, "        \t:   NONE    \n");          
    }

}

static void init_device_type_info(struct hct_type_info * pdevice, void *used_info, int type)
{
       memset(pdevice,0, sizeof(struct hct_type_info));
       INIT_LIST_HEAD(&pdevice->hct_device);
       INIT_LIST_HEAD(&pdevice->hct_dinfo);
       pdevice ->dev_type = type;
       if(used_info)
          pdevice->current_used_dinfo==used_info;
       
       if(type == ID_LCM_TYPE)
       {
          pdevice->type_print = hct_lcm_info_print;
          pdevice->set_used= hct_lcm_set_used;
       }
       else if(type == ID_CAMERA_TYPE)
       {
           pdevice->type_print = hct_camera_info_print;
           pdevice->set_used = hct_camera_set_used;
       }
       else if(type == ID_TOUCH_TYPE)
       {
           pdevice->type_print = hct_touchpanel_info_print;
           pdevice->set_used = hct_touchpanel_set_used;
       }
       else if(type == ID_ACCSENSOR_TYPE)
       {
           pdevice->type_print = hct_accsensor_info_print;
           pdevice->set_used = hct_accsensor_set_used;
       }
          
       mutex_init(&pdevice->info_mutex);
       list_add(&pdevice->hct_device,&hct_devices->type_list);
}

static void init_lcm_device(struct hct_lcm_device_idnfo *pdevice, LCM_DRIVER* nLcm)
{
    memset(pdevice,0, sizeof(struct hct_lcm_device_idnfo));
    INIT_LIST_HEAD(&pdevice->hct_list);
    INIT_LIST_HEAD(&pdevice->lcm_list);
    strcpy(pdevice->lcm_module_descrip,nLcm->name);
    pdevice->pdata=nLcm;
    
}

int hct_lcm_set_used(char * module_name, int pdata)
{
    struct hct_type_info * lcm_type_info;    
    struct hct_lcm_device_idnfo * plcm_device;
    int flag = -1;
    int reterror=0;
    

     list_for_each_entry(lcm_type_info,&hct_devices->type_list,hct_device){
             if(lcm_type_info->dev_type == ID_LCM_TYPE)
            {
                flag =1;  // this mean type is ok
                break;
            }
        }
     
     if(flag == -1)  // this mean type is new
     {
        reterror = -1;
        goto error_notype;
     }

    flag =-1;

    list_for_each_entry(plcm_device,&lcm_type_info->hct_dinfo,lcm_list){
           if(!strcmp(module_name,plcm_device->lcm_module_descrip))
           {
               plcm_device->type = DEVICE_USED;
               flag =1;  // this mean device is ok
               break;
           }
       }
    
    if(flag == 1)
        return 0;
    
error_notype:
    return reterror;
    
}

int hct_lcm_device_add(LCM_DRIVER* nLcm, campatible_type isUsed)
{
    struct hct_type_info * lcm_type_info;    
    struct hct_lcm_device_idnfo * plcm_device;
    int flag = -1;
    int reterror=0;
    

     list_for_each_entry(lcm_type_info,&hct_devices->type_list,hct_device){
             if(lcm_type_info->dev_type == ID_LCM_TYPE)
            {
                flag =1;  // this mean type is ok
                break;
            }
        }

       if(flag == -1)  // this mean type is new
       {
           lcm_type_info = kmalloc(sizeof(struct hct_type_info), GFP_KERNEL);

           if(lcm_type_info == NULL)
            {
                  printk("lcm_alloc type info failed ~~~~ \n");
                  reterror = -1;
                  goto malloc_faid;
            }
           
           if(isUsed)
             init_device_type_info(lcm_type_info, nLcm, ID_LCM_TYPE);
           else
             init_device_type_info(lcm_type_info, NULL, ID_LCM_TYPE);
       }
       else
       {
           if(isUsed &&(lcm_type_info->current_used_dinfo!=NULL))
             printk("~~~~add lcm error , duplicated current used lcm \n");
       }
     
       flag =-1;
       
       list_for_each_entry(plcm_device,&lcm_type_info->hct_dinfo,lcm_list){
              if(!strcmp(nLcm->name,plcm_device->lcm_module_descrip))
              {
                  flag =1;  // this mean device is ok
                  break;
              }
          }
       if(flag ==1)
       {
             printk("error ___ lcm type is duplicated \n");
             goto duplicated_faild;
       }
       else
       {
           plcm_device = kmalloc(sizeof(struct hct_lcm_device_idnfo), GFP_KERNEL);
            if(plcm_device == NULL)
            {
                  printk("lcm_alloc type info failed ~~~~ \n");
                  reterror = -2;
                  goto devicemalloc_faid;
            }
            
           init_lcm_device(plcm_device,nLcm);
           plcm_device->type=isUsed;
           
           
           list_add(&plcm_device->hct_list, &hct_devices->dev_list);
           list_add(&plcm_device->lcm_list,&lcm_type_info->hct_dinfo);
       }
 
     
     return 0;
duplicated_faild:
devicemalloc_faid:
    kfree(plcm_device);
malloc_faid:
    
    printk("%s: error return: %x: ---\n",__func__,reterror);
    return reterror;
    
}


static int hct_camera_set_used(char * module_name, int pdata)
{
    struct hct_type_info * camera_type_info;    
    struct hct_camera_device_dinfo * pcamera_device;
    int flag = -1;
    int reterror=0;
    

     list_for_each_entry(camera_type_info,&hct_devices->type_list,hct_device){
             if(camera_type_info->dev_type == ID_CAMERA_TYPE)
            {
                flag =1;  // this mean type is ok
                break;
            }
        }
     
     if(flag == -1)  // this mean type is new
     {
        reterror = -1;
        goto error_notype;
     }

    flag =-1;

    list_for_each_entry(pcamera_device,&camera_type_info->hct_dinfo,camera_list){
           if(!strcmp(module_name,pcamera_device->camera_module_descrip))
           {
               pcamera_device->type = DEVICE_USED;
               pcamera_device->cam_type = pdata;
               flag =1;  // this mean device is ok
               break;
           }
       }
    
    if(flag == 1)
        return 0;
    
error_notype:
    return reterror;
    
}

static int hct_camera_info_print(struct seq_file *se_f)
{
    
    struct hct_type_info * camera_type_info;    
    struct hct_camera_device_dinfo * pcamera_device;
    int flag = -1;

    seq_printf(se_f, "---------CAMERA USAGE-------- \n");

    list_for_each_entry(camera_type_info,&hct_devices->type_list,hct_device){
            if(camera_type_info->dev_type == ID_CAMERA_TYPE)
           {
               flag =1;  // this mean type is ok
               break;
           }
       }
    if(flag == 1)
    {
              list_for_each_entry(pcamera_device,&camera_type_info->hct_dinfo,camera_list){
                seq_printf(se_f, "      %20s\t\t:   ",pcamera_device->camera_module_descrip);
                if(pcamera_device->type == DEVICE_SUPPORTED)
                   seq_printf(se_f, "  supported \n");
                else
                {
                    seq_printf(se_f, "  used     \t");
                    if(pcamera_device->cam_type ==DUAL_CAMERA_MAIN_SENSOR)
                        seq_printf(se_f, "  main camera \n");
                    else if(pcamera_device->cam_type ==DUAL_CAMERA_SUB_SENSOR)
                        seq_printf(se_f, "  sub camera \n");
                    else
                        seq_printf(se_f, " camera unsupportd type \n");
                }
                
                
                }
    }
    else
    {
        seq_printf(se_f, "        \t:   NONE    \n");          
    }

}

static void init_camera_device(struct hct_camera_device_dinfo *pdevice, ACDK_KD_SENSOR_INIT_FUNCTION_STRUCT* mCamera)
{
    memset(pdevice,0, sizeof(struct hct_camera_device_dinfo));
    INIT_LIST_HEAD(&pdevice->hct_list);
    INIT_LIST_HEAD(&pdevice->camera_list);
    strcpy(pdevice->camera_module_descrip,mCamera->drvname);
    pdevice->pdata=mCamera;
    
}


int hct_camera_device_add(ACDK_KD_SENSOR_INIT_FUNCTION_STRUCT* mCamera, campatible_type isUsed)
{
    struct hct_type_info * camera_type_info;    
    struct hct_camera_device_dinfo * pcamera_device;
    int flag = -1;
    int reterror=0;

     list_for_each_entry(camera_type_info,&hct_devices->type_list,hct_device){
             if(camera_type_info->dev_type == ID_CAMERA_TYPE)
            {
                flag =1;  // this mean type is ok
                break;
            }
        }

       if(flag == -1)  // this mean type is new
       {
           camera_type_info = kmalloc(sizeof(struct hct_type_info), GFP_KERNEL);
           
           if(camera_type_info == NULL)
            {
                  printk("lcm_alloc type info failed ~~~~ \n");
                  reterror = -1;
                  goto malloc_faid;
            }
       
           if(isUsed)
             init_device_type_info(camera_type_info, mCamera, ID_CAMERA_TYPE);
           else
             init_device_type_info(camera_type_info, NULL, ID_CAMERA_TYPE);
       }
       else
       {
           if(isUsed &&(camera_type_info->current_used_dinfo!=NULL))
             printk("~~~~add camera error , duplicated current used lcm \n");
       }
     
       flag =-1;
       
       list_for_each_entry(pcamera_device,&camera_type_info->hct_dinfo,camera_list){
              if(!strcmp(mCamera->drvname,pcamera_device->camera_module_descrip))
              {
                  flag =1;  // this mean device is ok
                  break;
              }
          }
       if(flag ==1)
       {
             goto duplicated_faild;
       }
       else
       {
           pcamera_device = kmalloc(sizeof(struct hct_camera_device_dinfo), GFP_KERNEL);
            if(camera_type_info == NULL)
            {
                  printk("lcm_alloc type info failed ~~~~ \n");
                  reterror = -2;
                  goto malloc_faid;
            }
           init_camera_device(pcamera_device,mCamera);
           pcamera_device->type=isUsed;
           
           list_add(&pcamera_device->hct_list, &hct_devices->dev_list);
           list_add(&pcamera_device->camera_list,&camera_type_info->hct_dinfo);
       }
 
          
          return 0;
          
     duplicated_faild:
     devicemalloc_faid:
         kfree(pcamera_device);
     malloc_faid:
         
         printk("%s: error return: %x: ---\n",__func__,reterror);
         return reterror;
    
}


static int hct_touchpanel_set_used(char * module_name, int pdata)
{
    struct hct_type_info * touchpanel_type_info;    
    struct hct_touchpanel_device_dinfo * ptouchpanel_device;
    int flag = -1;
    int reterror=0;
    

     list_for_each_entry(touchpanel_type_info,&hct_devices->type_list,hct_device){
             if(touchpanel_type_info->dev_type == ID_TOUCH_TYPE)
            {
                printk("touch type has find break !!\n");
                flag =1;  // this mean type is ok
                break;
            }
        }
     
     if(flag == -1)  // this mean type is new
     {
        reterror = -1;
        goto error_notype;
     }

    flag =-1;

    list_for_each_entry(ptouchpanel_device,&touchpanel_type_info->hct_dinfo,touch_list){
           if(!strcmp(module_name,ptouchpanel_device->touch_descrip))
           {
               ptouchpanel_device->type = DEVICE_USED;
               flag =1;  // this mean device is ok
               break;
           }
       }
    
    if(flag == 1)
        return 0;
    
error_notype:
    return reterror;
    
}

static int hct_touchpanel_info_print(struct seq_file *se_f)
{
    
    struct hct_type_info * touchpanel_type_info;    
    struct hct_touchpanel_device_dinfo * ptouchpanel_device;
    int flag = -1;

    seq_printf(se_f, "---------TOUCHPANEL USAGE---------\t \n");

    list_for_each_entry(touchpanel_type_info,&hct_devices->type_list,hct_device){
            if(touchpanel_type_info->dev_type == ID_TOUCH_TYPE)
           {
               flag =1;  // this mean type is ok
               break;
           }
       }
    if(flag == 1)
    {
              list_for_each_entry(ptouchpanel_device,&touchpanel_type_info->hct_dinfo,touch_list){
                seq_printf(se_f, "      %20s\t\t:   ",ptouchpanel_device->touch_descrip);
                if(ptouchpanel_device->type == DEVICE_SUPPORTED)
                   seq_printf(se_f, "  supported \n");
                else
                {
                    seq_printf(se_f, "  used \t\n");
                }
                
                }
    }
    else
    {
        seq_printf(se_f, "        \t:   NONE    \n");          
    }

}

static void init_touchpanel_device(struct hct_touchpanel_device_dinfo *pdevice, struct tpd_driver_t * mTouch)
{
    memset(pdevice,0, sizeof(struct hct_touchpanel_device_dinfo));
    INIT_LIST_HEAD(&pdevice->hct_list);
    INIT_LIST_HEAD(&pdevice->touch_list);
    strcpy(pdevice->touch_descrip,mTouch->tpd_device_name);
    pdevice->pdata=mTouch;
    
}


int hct_touchpanel_device_add(struct tpd_driver_t* mTouch, campatible_type isUsed)
{
    struct hct_type_info * touchpanel_type_info;    
    struct hct_touchpanel_device_dinfo * ptouchpanel_device;
    int flag = -1;
    int reterror=0;

     list_for_each_entry(touchpanel_type_info,&hct_devices->type_list,hct_device){
             if(touchpanel_type_info->dev_type == ID_TOUCH_TYPE)
            {
                flag =1;  // this mean type is ok
                break;
            }
        }

       if(flag == -1)  // this mean type is new
       {
           touchpanel_type_info = kmalloc(sizeof(struct hct_type_info), GFP_KERNEL);

           if(touchpanel_type_info == NULL)
            {
                  printk("lcm_alloc type info failed ~~~~ \n");
                  reterror = -1;
                  goto malloc_faid;
            }
       
           if(isUsed)
             init_device_type_info(touchpanel_type_info, mTouch, ID_TOUCH_TYPE);
           else
             init_device_type_info(touchpanel_type_info, NULL, ID_TOUCH_TYPE);
       }
       else
       {
           if(isUsed &&(touchpanel_type_info->current_used_dinfo!=NULL))
             printk("~~~~add lcm error , duplicated current used lcm \n");
       }
     
       flag =-1;
       
       list_for_each_entry(ptouchpanel_device,&touchpanel_type_info->hct_dinfo,touch_list){
              if(!strcmp(mTouch->tpd_device_name,ptouchpanel_device->touch_descrip))
              {
                  flag =1;  // this mean device is ok
                  break;
              }
          }
       if(flag ==1)
       {
             goto duplicated_faild;
       }
       else
       {
           ptouchpanel_device = kmalloc(sizeof(struct hct_touchpanel_device_dinfo), GFP_KERNEL);
            if(ptouchpanel_device == NULL)
            {
                  printk("lcm_alloc type info failed ~~~~ \n");
                  reterror = -2;
                  goto malloc_faid;
            }
           init_touchpanel_device(ptouchpanel_device,mTouch);
           ptouchpanel_device->type=isUsed;
           
           list_add(&ptouchpanel_device->hct_list, &hct_devices->dev_list);
           list_add(&ptouchpanel_device->touch_list,&touchpanel_type_info->hct_dinfo);
       }
 
          
          return 0;
          
     duplicated_faild:
     devicemalloc_faid:
         kfree(ptouchpanel_device);
     malloc_faid:
         
         printk("%s: error return: %x: ---\n",__func__,reterror);
         return reterror;
    
}


static int hct_accsensor_set_used(char * module_name, int pdata)
{
    struct hct_type_info * accsensor_type_info;    
    struct hct_accsensor_device_dinfo * paccsensor_device;
    int flag = -1;
    int reterror=0;
    

     list_for_each_entry(accsensor_type_info,&hct_devices->type_list,hct_device){
             if(accsensor_type_info->dev_type == ID_ACCSENSOR_TYPE)
            {
                flag =1;  // this mean type is ok
                break;
            }
        }
     
     if(flag == -1)  // this mean type is new
     {
        reterror = -1;
        goto error_notype;
     }

    flag =-1;

    list_for_each_entry(paccsensor_device,&accsensor_type_info->hct_dinfo,sensor_list){
           if(!strcmp(module_name,paccsensor_device->sensor_descrip))
           {
               paccsensor_device->type = DEVICE_USED;
               flag =1;  // this mean device is ok
               break;
           }
       }
    
    if(flag == 1)
        return 0;
    
error_notype:
    return reterror;
    
}

static int hct_accsensor_info_print(struct seq_file *se_f)
{
    
    struct hct_type_info * accsensor_type_info;    
    struct hct_accsensor_device_dinfo * paccsensor_device;
    int flag = -1;

    seq_printf(se_f, "---------ACCSENSOR USAGE---------\t \n");
    
#if defined(MTK_AUTO_DETECT_ACCELEROMETER)

    list_for_each_entry(accsensor_type_info,&hct_devices->type_list,hct_device){
            if(accsensor_type_info->dev_type == ID_ACCSENSOR_TYPE)
           {
               flag =1;  // this mean type is ok
               break;
           }
       }
    if(flag == 1)
    {
              list_for_each_entry(paccsensor_device,&accsensor_type_info->hct_dinfo,sensor_list){
                seq_printf(se_f, "      %20s\t\t:   ",paccsensor_device->sensor_descrip);
                if(paccsensor_device->type == DEVICE_SUPPORTED)
                   seq_printf(se_f, "  supported \n");
                else
                {
                    seq_printf(se_f, "  used \t\n");
                }
                }
    }
    else
    {
        seq_printf(se_f, "        \t:   NONE    \n");          
    }
#else
    seq_printf(se_f, " warring :MTK_AUTO_DETECT_ACCELEROMETER need set to yes to display  \n");          
#endif
}

static void init_accsensor_device(struct hct_accsensor_device_dinfo *pdevice, struct sensor_init_info * maccsensor)
{
    memset(pdevice,0, sizeof(struct hct_accsensor_device_dinfo));
    INIT_LIST_HEAD(&pdevice->hct_list);
    INIT_LIST_HEAD(&pdevice->sensor_list);
    strcpy(pdevice->sensor_descrip,maccsensor->name);
    pdevice->pdata=maccsensor;
    
}


int hct_accsensor_device_add(struct sensor_init_info* maccsensor, campatible_type isUsed)
{
    struct hct_type_info * accsensor_type_info;    
    struct hct_accsensor_device_dinfo * paccsensor_device;
    int flag = -1;
    int reterror=0;

     list_for_each_entry(accsensor_type_info,&hct_devices->type_list,hct_device){
             if(accsensor_type_info->dev_type == ID_ACCSENSOR_TYPE)
            {
                flag =1;  // this mean type is ok
                break;
            }
        }

       if(flag == -1)  // this mean type is new
       {
           accsensor_type_info = kmalloc(sizeof(struct hct_type_info), GFP_KERNEL);

           if(accsensor_type_info == NULL)
            {
                  printk("lcm_alloc type info failed ~~~~ \n");
                  reterror = -1;
                  goto malloc_faid;
            }
       
           if(isUsed)
             init_device_type_info(accsensor_type_info, maccsensor, ID_ACCSENSOR_TYPE);
           else
             init_device_type_info(accsensor_type_info, NULL, ID_ACCSENSOR_TYPE);
       }
       else
       {
           if(isUsed &&(accsensor_type_info->current_used_dinfo!=NULL))
             printk("~~~~add lcm error , duplicated current used lcm \n");
       }
     
       flag =-1;
       
       list_for_each_entry(paccsensor_device,&accsensor_type_info->hct_dinfo,sensor_list){
              if(!strcmp(maccsensor->name,paccsensor_device->sensor_descrip))
              {
                  flag =1;  // this mean device is ok
                  break;
              }
          }
       if(flag ==1)
       {
             goto duplicated_faild;
       }
       else
       {
           paccsensor_device = kmalloc(sizeof(struct hct_accsensor_device_dinfo), GFP_KERNEL);
            if(paccsensor_device == NULL)
            {
                  printk("lcm_alloc type info failed ~~~~ \n");
                  reterror = -2;
                  goto malloc_faid;
            }
           init_accsensor_device(paccsensor_device,maccsensor);
           paccsensor_device->type=isUsed;
           
           list_add(&paccsensor_device->hct_list, &hct_devices->dev_list);
           list_add(&paccsensor_device->sensor_list,&accsensor_type_info->hct_dinfo);
       }
 
          
          return 0;
          
     duplicated_faild:
     devicemalloc_faid:
         kfree(paccsensor_device);
     malloc_faid:
         
         printk("%s: error return: %x: ---\n",__func__,reterror);
         return reterror;
    
}


static int hct_set_device_used(int dev_type, char * module_name, int pdata)
{
    struct hct_type_info * type_info;    
    int ret_val = 0;
    
    list_for_each_entry(type_info,&hct_devices->type_list,hct_device){
        if(type_info->dev_type == dev_type)
        {
            if(type_info->set_used!=NULL)
                type_info->set_used(module_name,pdata);
        }
    }
    
    return ret_val;
}

int hct_set_touch_device_used(char * module_name, int pdata)
{
    int ret_val = 0;
    ret_val = hct_set_device_used(ID_TOUCH_TYPE,module_name,pdata);
    return ret_val;
}


int hct_set_camera_device_used(char * module_name, int pdata)
{
    int ret_val = 0;
    ret_val = hct_set_device_used(ID_CAMERA_TYPE,module_name,pdata);
    return ret_val;
}

int hct_set_accsensor_device_used(char * module_name, int pdata)
{
    int ret_val = 0;
    ret_val = hct_set_device_used(ID_ACCSENSOR_TYPE,module_name,pdata);
    return ret_val;
}


int hct_device_dump(struct seq_file *m)
{
    
    struct hct_type_info * type_info;    
    seq_printf(m," \t\tDEVICE DUMP--begin\n");
    
    list_for_each_entry(type_info,&hct_devices->type_list,hct_device){
        if(type_info->type_print == NULL)
        {
            printk(" error !!! this type [%d] has no dump fun \n",type_info->dev_type);
        }
        else
            type_info->type_print(m);
    }
    seq_printf(m," \t\tDEVICE DUMP--end\n");
}

extern LCM_DRIVER* lcm_driver_list[];
extern unsigned int lcm_count;
extern char mtkfb_lcm_name[];
extern int  proc_hctinfo_init(void);
extern ACDK_KD_SENSOR_INIT_FUNCTION_STRUCT kdSensorList[];
extern const LCM_DRIVER  *lcm_drv;

static int hct_devices_probe(struct platform_device *pdev) 
{
        int temp;
        int err =0;
        
        err = proc_hctinfo_init();
        if(err<0)
            goto proc_error;
        
        hct_devices = kmalloc(sizeof(struct hct_devices_info), GFP_KERNEL);
        
        if(hct_devices == NULL)
        {
             printk("%s: error probe becase of mem\n",__func__);
             return -1;
        }
        
        INIT_LIST_HEAD(&hct_devices->type_list);
        INIT_LIST_HEAD(&hct_devices->dev_list);
        mutex_init(&hct_devices->de_mutex);
        
        if(lcm_count ==1)
        {
            hct_lcm_device_add(lcm_driver_list[0], DEVICE_USED);
        }
        else
        {
            for(temp= 0;temp< lcm_count;temp++)
            {
                if(lcm_drv == lcm_driver_list[temp])                    
                    hct_lcm_device_add(lcm_driver_list[temp], DEVICE_USED);
                else
                    hct_lcm_device_add(lcm_driver_list[temp], DEVICE_SUPPORTED);
                    
            }
        }

       for(temp=0;;temp++)
       {
           if(kdSensorList[temp].SensorInit!=NULL)
           {
               hct_camera_device_add(&kdSensorList[temp], DEVICE_SUPPORTED);
           }
           else
            break;
       }

	return 0;
 proc_error:
       return err;
}
/*----------------------------------------------------------------------------*/
static int hct_devices_remove(struct platform_device *pdev)
{
	return 0;
}


/*----------------------------------------------------------------------------*/
static struct platform_driver hct_devices_driver = {
	.probe      = hct_devices_probe,
	.remove     = hct_devices_remove,    
	.driver     = {
		.name  = "hct_devices",
		//.owner = THIS_MODULE,
	}
};

static struct platform_device hct_dev = {
	.name		  = "hct_devices",
	.id		  = -1,
};


/*----------------------------------------------------------------------------*/
static int __init hct_devices_init(void)
{
    int retval = 0;
    
    retval = platform_device_register(&hct_dev);
    if (retval != 0){
        return retval;
    }

    if(platform_driver_register(&hct_devices_driver))
    {
    	printk("failed to register driver");
    	return -ENODEV;
    }
    
    return 0;
}
/*----------------------------------------------------------------------------*/
static void __exit hct_devices_exit(void)
{
	platform_driver_unregister(&hct_devices_driver);
}
/*----------------------------------------------------------------------------*/
module_init(hct_devices_init);
module_exit(hct_devices_exit);
/*----------------------------------------------------------------------------*/
MODULE_AUTHOR("Jay Zhou");
MODULE_DESCRIPTION("HCT DEVICE INFO");
MODULE_LICENSE("GPL");




