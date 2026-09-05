#ifndef PORTIO_H
#define PORTIO_H

void pout(unsigned short port, unsigned char value);
unsigned char pin(unsigned short port);
void pout16(unsigned short port, unsigned short value);
unsigned short pin16(unsigned short port);

#endif
