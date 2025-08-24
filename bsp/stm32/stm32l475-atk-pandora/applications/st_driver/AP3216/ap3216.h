
#ifndef __DRV__AP3216_H
#define __DRV__AP3216_H
 
#ifdef __cplusplus
extern "C" {
#endif
 
#define AP3216C_ADDR          0X1E
 
/* AP3316C寄存器 */
#define AP3216C_SYSTEMCONG    0x00    /* 配置寄存器       */
#define AP3216C_INTSTATUS     0X01    /* 中断状态寄存器   */
#define AP3216C_INTCLEAR      0X02    /* 中断清除寄存器   */
#define AP3216C_IRDATALOW     0x0A    /* IR数据低字节     */
#define AP3216C_IRDATAHIGH    0x0B    /* IR数据高字节     */
#define AP3216C_ALSDATALOW    0x0C    /* ALS数据低字节    */
#define AP3216C_ALSDATAHIGH   0X0D    /* ALS数据高字节    */
#define AP3216C_PSDATALOW     0X0E    /* PS数据低字节     */
#define AP3216C_PSDATAHIGH    0X0F    /* PS数据高字节     */
 
typedef struct _ap3216c_data{
   unsigned short ir; 
   unsigned short als; 
   unsigned short ps;      
}ap3216c_data;
 
void ap3216c_read_datas(ap3216c_data *pdata);
int ap3216c_init(void);
 
 
#ifdef __cplusplus
}
#endif
 
#endif /* __DRV__AP3216_H */


