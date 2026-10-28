#ifndef MYMILLI_H
#define MYMILLI_H

#define MY_TCCR2A (*(volatile unsigned char *)0xB0)
#define MY_TCCR2B (*(volatile unsigned char *)0xB1)
#define MY_TCNT2  (*(volatile unsigned char *)0xB2)
#define MY_TIMSK2 (*(volatile unsigned char *)0x70)

#define MY_SREG   (*(volatile unsigned char *)0x5F)

volatile unsigned long my_timer2_millis = 0;
volatile unsigned int my_timer2_fract = 0;


extern "C" void __vector_9(void) __attribute__ ((signal, used, externally_visible));
void __vector_9(void) {
  unsigned long m = my_timer2_millis;
  unsigned int f = my_timer2_fract;

  m += 1;
  f += 24;

  if (f >= 1000) {
    f -= 1000;
    m += 1;
  }
  
  my_timer2_fract = f;
  my_timer2_millis = m;
}


unsigned long myMilli(void) {
  unsigned long m;
  
  unsigned char oldSREG = MY_SREG; 
  MY_SREG &= ~(1 << 7);
  m = my_timer2_millis;
  MY_SREG = oldSREG;
  
  return m;
}

#endif