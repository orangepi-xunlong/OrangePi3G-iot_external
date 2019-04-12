#include <sys/types.h>
#include <stdint.h>

#include <platform/env.h>
#include <mt_partition.h>
#include <platform/mt_typedefs.h>
#include <platform/errno.h>

#define MODULE_NAME "LK_ENV"
env_t g_env;
static int env_valid = 0;
static char *env_buffer = NULL;
static char env_get_char(int index);
static char *env_get_addr(int index);
static int envmatch (uchar *s1, int i2);
static int write_env_area(char *env_buf);
static int read_env_area(char *env_buf);
extern int mboot_common_get_logo_flag();



void env_init(void)
{
	char *env_buf;
	int ret,i;
	env_buffer = (char *)malloc(CFG_ENV_SIZE);
	memset(env_buffer,0x00,CFG_ENV_SIZE);	
	g_env.env_data = env_buffer + CFG_ENV_DATA_OFFSET;
	ret = read_env_area(env_buffer);
	int checksum = 0;
	if(ret<0)
	{
		printf("[%s]read_env_area fail, ret = %x\n",MODULE_NAME,ret);
		env_valid = 0;
		goto end;
	}

	memcpy(g_env.sig,env_buffer,sizeof(g_env.sig));
	memcpy(g_env.sig_1,env_buffer+CFG_ENV_SIG_1_OFFSET,sizeof(g_env.sig_1));
	
	if(!strcmp(g_env.sig,ENV_SIG) && !strcmp(g_env.sig_1,ENV_SIG)){		
		g_env.checksum = *((int *)env_buffer+CFG_ENV_CHECKSUM_OFFSET/4);
		for(i=0;i<(CFG_ENV_DATA_SIZE);i++){
			checksum += g_env.env_data[i];
		}
		if(checksum != g_env.checksum){
			printf("[%s]checksum mismatch s %d d %d!\n",MODULE_NAME,g_env.checksum,checksum);
			env_valid = 0;
			goto end;
		}else{
			printf("[%s]ENV initialize sucess\n",MODULE_NAME);
			env_valid = 1;
		}
		
	}else{
		printf("[%s]ENV SIG Wrong\n",MODULE_NAME);
		env_valid = 0;
		goto end;
	}
end:
	if(!env_valid){
		memset(env_buffer,0x00,CFG_ENV_SIZE);
	}
	
}

char *get_env(char *name)
{
	int i, nxt;
	printf("[%s]get_env %s\n",MODULE_NAME,name);
	if(!env_valid)
		return NULL;

	for (i=0; env_get_char(i) != '\0'; i=nxt+1) {
		int val;

		for (nxt=i; env_get_char(nxt) != '\0'; ++nxt) {
			if (nxt >= CFG_ENV_SIZE) {
				return (NULL);
			}
		}
		if ((val=envmatch((uchar *)name, i)) < 0)
			continue;
		return ((char *)env_get_addr(val));
	}

	return (NULL);
}
static char env_get_char(int index)
{
	return *(g_env.env_data+index);
}

static char *env_get_addr(int index)
{
	return (g_env.env_data+index);

}
static int envmatch (uchar *s1, int i2)
{

	while (*s1 == env_get_char(i2++))
		if (*s1++ == '=')
			return(i2);
	if (*s1 == '\0' && env_get_char(i2-1) == '=')
		return(i2);
	return(-1);
}


int set_env(char *name,char *value)
{
	int   i, len, oldval;
	uchar *env, *nxt = NULL;
	
	int ret;

	uchar *env_data = g_env.env_data;

	printf("[%s]set_env %s %s\n",MODULE_NAME,name,value);

	oldval = -1;

	if(!env_valid){
		env = env_data;
		goto add;
	}
	
	for (env=env_data; *env; env=nxt+1) {
		for (nxt=env; *nxt; ++nxt)
			;
		if ((oldval = envmatch((uchar *)name, env-env_data)) >= 0)
			break;
	}

	if(oldval>0){
		if (*++nxt == '\0') {
			if (env > env_data) {
				env--;
			} else {
				*env = '\0';
			}
		} else {
			for (;;) {
				*env = *nxt++;
				if ((*env == '\0') && (*nxt == '\0'))
					break;
				++env;
			}
		}
		*++env = '\0';
	}

	for (env=env_data; *env || *(env+1); ++env)
		;
	if (env > env_data)
		++env;

add:
		/*
	 * Overflow when:
	 * "name" + "=" + "val" +"\0\0"  > ENV_SIZE - (env-env_data)
	 */
	len = strlen(name) + 2;
	/* add '=' for first arg, ' ' for all others */
	len += strlen(value) + 1;

	if (len > (&env_data[CFG_ENV_DATA_SIZE]-env)) {
		printf ("## Error: environment overflow, \"%s\" deleted\n", name);
		return -1;
	}
	while ((*env = *name++) != '\0')
		env++;
	
	*env = '=';
	
	while ((*++env = *value++) != '\0')
			;


	/* end is marked with double '\0' */
	*++env = '\0';
	memset(env,0x00,CFG_ENV_DATA_SIZE-(env-env_data));
	ret = write_env_area(env_buffer);
	if(ret < 0){
		printf("[%s]write env fail\n",MODULE_NAME);
		memset(env_buffer,0x00,CFG_ENV_SIZE);
		return -1;
	}
	env_valid = 1;
	return 0;

}

void print_env()
{
	int i,nxt;
	uchar *env = g_env.env_data;
	if(!env_valid){
		printf("[%s]no valid env\n",MODULE_NAME);
		return;
	}
	printf("[%s]env:\n",MODULE_NAME);
	for (i=0; env_get_char(i) != '\0'; i=nxt+1) {
		for (nxt=i; env_get_char(nxt) != '\0'; ++nxt) {
			if (nxt >= (CFG_ENV_DATA_SIZE)) {
				return;
			}
		}
		printf("%s\n",env+i);
	}

	
	
}
static int write_env_area(char *env_buf)
{
	part_t *part;	
    part_dev_t *dev;

	long len;
	int i,checksum = 0;
#ifdef MTK_EMMC_SUPPORT
	unsigned long long start_addr;
#else
	unsigned long start_addr; 
#endif

	dev = mt_part_get_device();
	if (!dev)
	{	return -ENODEV;
	}

	part = mt_part_get_partition(ENV_PART);
	if (!part)
	{	return -ENOENT;
	}

#ifdef MTK_EMMC_SUPPORT
	start_addr = (u64)part->startblk * BLK_SIZE; 
#else
	start_addr = part->startblk * BLK_SIZE;    
#endif 

	memcpy(env_buf,ENV_SIG,sizeof(g_env.sig));
	memcpy(env_buf+CFG_ENV_SIG_1_OFFSET,ENV_SIG,sizeof(g_env.sig));

	for(i=0;i<CFG_ENV_DATA_SIZE;i++){
		checksum += *(env_buf+CFG_ENV_DATA_OFFSET+i);
	}
	printf("checksum %d\n",checksum);

	*((int *)env_buf+CFG_ENV_CHECKSUM_OFFSET/4) = checksum;

	
	len = dev->write(dev, (uchar*)env_buf, start_addr + CFG_ENV_OFFSET, CFG_ENV_SIZE);	 
	if (len < 0) {
		return -EIO;
	}

	return 0;

}


int mboot_common_get_logo_flag()
{
  long len;
#ifdef MTK_EMMC_SUPPORT
  unsigned long long start_addr;
  unsigned long long length;
#else
  unsigned long start_addr;
  unsigned long length;
#endif    

  part_t *part;
  part_dev_t *dev;
  char part_name[]="PRO_INFO";
  length=4096;
  printf("[JVX] Read logo flag from pro_info\n");
  dev = mt_part_get_device();
  if (!dev)
  {
   return -ENODEV;
  }
  printf("[JVM]Read PRO_INFO 1\n");
  part=mt_part_get_partition(part_name);
  if (!part)
  {
   return -ENOENT;
  }
  printf("[JVM] Read PRO_INFO 1\n");
  printf("[JXM] Part info name:%s\n",part->name);
  printf("[JVM] Part info startblk:%ld\n",part->startblk);
  printf("[JVM] Part info part blks: %ld\n",part->blknum);
#ifdef MTK_EMMC_SUPPORT
  start_addr =(u64)(part->statblk)*BLK_SIZE;
  length =(u64) part->blknum*BLK_SIZE;
#else
  start_addr =part->startblk*BLK_SIZE;
  length =part->blknum*BLK_SIZE;
  
#endif
  printf("[JVM]Read PRO_INFO 3\n");


  printf("[JVM]Read the data of %s\n",part_name);
  unsigned char*addr;
  length=10;//4096
  unsigned char i=0;
  addr =(unsigned char*)malloc(length*sizeof(unsigned char));
  if (!addr)
  {
    printf("malloc error\r\n");
  }
  	
  /*for(i=0;i++;i<128)
  {
	*(addr+i)=i;
  }

  // printf("[JVM]The last number0:%d\n",*(addr+10));
  
   for(int j=0;j<10;j++)
   {
	   printf("[JVM]resul0t:i:%d---%d--\n",j+1,*(addr+j));
	
   }


   len=dev->write(dev,(uchar*)addr,start_addr,length);
  if (len<0)
  {
   len=-EIO;
   goto exit;
  
  }*/
  memset(addr,0,length*sizeof(unsigned char));
  len=dev->read(dev,start_addr,(uchar*)addr,length);
  if (len<0)
  {
   len=-EIO;
   goto exit;
  
  }
 
  printf("[JVM]The last number:%d\n",*(addr+9));
  
  for(int j=0;j<10;j++)
  {
   printf("[JVM]result:i:%d---%d--\n",j+1,*(addr+j));
   //addr++;
  }
exit:
   //if(addr)
   //	free(addr);
   
  return *(addr);
}






static int read_env_area(char *env_buf)
{
	part_t *part;	
    part_dev_t *dev;
	long len;
#ifdef MTK_EMMC_SUPPORT
	unsigned long long start_addr;
#else
	unsigned long start_addr; 
#endif

	dev = mt_part_get_device();
    if (!dev)
    {	return -ENODEV;
    }

    part = mt_part_get_partition(ENV_PART);
    if (!part)
    {	return -ENOENT;
    }

#ifdef MTK_EMMC_SUPPORT
	start_addr = (u64)part->startblk * BLK_SIZE; 
#else
    start_addr = part->startblk * BLK_SIZE;    
#endif 

	len = dev->read(dev, start_addr + CFG_ENV_OFFSET, (uchar*)env_buf, CFG_ENV_SIZE);    
    if (len < 0) {
		return -EIO;
    }
	return 0;
}
