#include <avr/io.h>

void udebug_init(void)
{
    //DDRC |= (1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3);
    //DDRC |= (1<<PC0) | (1<<PC1);
    //PORTC &= ~((1<<PC0) | (1<<PC1) | (1<<PC2) | (1<<PC3)); 
    //PORTC &= ~((1<<PC0) | (1<<PC1)); 
    //PORTC |= (1<<PC2);
}

inline void udebug_edge(void)
{
   //PORTC ^= (1<<PC0); 
}

//inline void udebug_
inline void udebug_txon(void)
{
    
   //PORTC |= (1<<PC1); 
}

inline void udebug_txoff(void)
{ 
   //PORTC &= ~(1<<PC1); 
}


inline void udebug_rx(void)
{ 
   //PORTC ^= (1<<PC2); 
}
