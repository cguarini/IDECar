# ifndef UART_H
# define UART_H
# include <stdint.h>
void put ( char * ptr_str );
void putnumU (int i );
void uart_init ( void );
uint8_t uart_getchar ( void );
void uart_putchar ( char ch );
void LED_init(void);
void SW3_INIT(void);
void SW2_INIT(void);
void PDB_INIT(void);

# endif /* ifndef UART_H */
