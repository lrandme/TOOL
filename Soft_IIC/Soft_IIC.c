#include "Soft_IIC.h"


#define SDA_OUT()       {                                                     \
                                DL_GPIO_initDigitalOutput(IIC_SDA_IOMUX);     \
                                DL_GPIO_setPins(IIC_PORT, IIC_SDA_PIN);       \
                                DL_GPIO_enableOutput(IIC_PORT, IIC_SDA_PIN);  \
                        }


#define SDA_IN()        { DL_GPIO_initDigitalInput(IIC_SDA_IOMUX); }


#define SDA_GET()         ( ( ( DL_GPIO_readPins(IIC_PORT,IIC_SDA_PIN) & IIC_SDA_PIN ) > 0 ) ? 1 : 0 )

#define SDA(x)           ( (x) ? (DL_GPIO_setPins(IIC_PORT,IIC_SDA_PIN)) : (DL_GPIO_clearPins(IIC_PORT,IIC_SDA_PIN)) )
#define SCL(x)            ( (x) ? (DL_GPIO_setPins(IIC_PORT,IIC_SCL_PIN)) : (DL_GPIO_clearPins(IIC_PORT,IIC_SCL_PIN)) )

#define IIC_DELAY(x)   delay_us( (x) )
#define ACK_TIMEOUT    1000

#define IIC_ACK     0
#define IIC_NACK    1

void Soft_IIC_Start(void){
        SDA_OUT();
        SCL(1);
        SDA(1);
        IIC_DELAY(5);

        /* 确保 SCL 已稳定为高 */
        NOP();          /* ≈ 100 ns @ 80 MHz */
        NOP();

        SDA(0);
        IIC_DELAY(5);  
        SCL(0);
}

void Soft_IIC_Stop(void){
        SDA_OUT();
        SCL(0);
        SDA(0);
        IIC_DELAY(5);
        SCL(1);
        /* 确保 SCL 已稳定为高 */
        NOP();
        NOP();
        IIC_DELAY(5);
        SDA(1);
        IIC_DELAY(5);
}


void Soft_IIC_Send_Ack(unsigned char ack){
        SDA_OUT();
        SCL(0);
        SDA(0);
        IIC_DELAY(5);
        if(!ack) SDA(0);
        else     SDA(1);
        SCL(1);
        IIC_DELAY(5);
        SCL(0);
        
}


unsigned char Soft_IIC_WaitAck(void){
        char ack = 0;
        uint16_t timeout = ACK_TIMEOUT;
        SCL(0);
        SDA(1);
        SDA_IN();
        
        SCL(1);
        while( (SDA_GET()==1) && ( timeout ) )
        {
                timeout--;
                IIC_DELAY(5);
        }
        
        if( timeout <= 0 )
        {
                Soft_IIC_Stop();
                return 1;
        }
        else
        {
                SCL(0);
                SDA_OUT();
        }
        return ack;
}


void Soft_IIC_Send_Byte(uint8_t dat){
        int i = 0;
        SDA_OUT();
        SCL(0);
        

        for( i = 0; i < 8; i++ )
        {
                SDA( (dat & 0x80) >> 7 );
                IIC_DELAY(1);
                SCL(1);
                IIC_DELAY(5);
                SCL(0);
                IIC_DELAY(5);
                dat<<=1;
        }        
}

unsigned char Soft_IIC_Read_Byte(void){
        unsigned char i,receive=0;
        SDA_IN();
    for(i=0;i<8;i++ )
        {
        SCL(0);
        IIC_DELAY(5);
        SCL(1);
        IIC_DELAY(5);
        receive<<=1;
        if( SDA_GET() )
        {        
            receive|=1;   
        }
        IIC_DELAY(5); 
    }                                         
        SCL(0); 
  return receive;
}


char Soft_IIC_WriteReg(uint8_t addr,uint8_t regaddr,uint8_t num,uint8_t *regdata){
    uint16_t i = 0;
        Soft_IIC_Start();
        Soft_IIC_Send_Byte((addr<<1)|0);
        if( Soft_IIC_WaitAck() == 1 ) {Soft_IIC_Stop();return 1;}
        Soft_IIC_Send_Byte(regaddr);
        if( Soft_IIC_WaitAck() == 1 ) {Soft_IIC_Stop();return 2;}
    
        for(i=0;i<num;i++)
    {
        Soft_IIC_Send_Byte(regdata[i]);
        if( Soft_IIC_WaitAck() == 1 ) {Soft_IIC_Stop();return (3+i);}
    }        
        Soft_IIC_Stop();
    return 0;
}

char Soft_IIC_ReadReg(uint8_t addr, uint8_t regaddr,uint8_t num,uint8_t* Read){
        uint8_t i;
        Soft_IIC_Start();
        Soft_IIC_Send_Byte((addr<<1)|0);
        if( Soft_IIC_WaitAck() == 1 ) {Soft_IIC_Stop();return 1;}
        Soft_IIC_Send_Byte(regaddr);
        if( Soft_IIC_WaitAck() == 1 ) {Soft_IIC_Stop();return 2;}
        
        Soft_IIC_Start();
        Soft_IIC_Send_Byte((addr<<1)|1);
        if( Soft_IIC_WaitAck() == 1 ) {Soft_IIC_Stop();return 3;}
        
        for(i=0;i<(num-1);i++){
                Read[i]=Soft_IIC_Read_Byte();
                Soft_IIC_Send_Ack( IIC_ACK );
        }
        Read[i]=Soft_IIC_Read_Byte();
        Soft_IIC_Send_Ack( IIC_NACK );         
        Soft_IIC_Stop();
        return 0;
}

