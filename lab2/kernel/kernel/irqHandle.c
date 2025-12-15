#include "x86.h"
#include "device.h"

extern int displayRow;
extern int displayCol;

extern uint32_t keyBuffer[MAX_KEYBUFFER_SIZE];
extern int bufferHead;
extern int bufferTail;

void GProtectFaultHandle(struct TrapFrame *tf);

void KeyboardHandle(struct TrapFrame *tf);

void syscallHandle(struct TrapFrame *tf);
void syscallWrite(struct TrapFrame *tf);
void syscallPrint(struct TrapFrame *tf);
void syscallRead(struct TrapFrame *tf);
void syscallGetChar(struct TrapFrame *tf);
void syscallGetStr(struct TrapFrame *tf);


void irqHandle(struct TrapFrame *tf) { // pointer tf = esp
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%es"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%fs"::"a"(KSEL(SEG_KDATA)));
	//asm volatile("movw %%ax, %%gs"::"a"(KSEL(SEG_KDATA)));
	switch(tf->irq) {
		// TODO: 填好中断处理程序的调用
    case -1:
      break;
    case 0xd:
      GProtectFaultHandle(tf);
      break;
    case 0x21:
      KeyboardHandle(tf);
      break;
    case 0x80:
      syscallHandle(tf);
      break;
		default:
      assert(0);
	}
}

void GProtectFaultHandle(struct TrapFrame *tf){
	assert(0);
	return;
}

void KeyboardHandle(struct TrapFrame *tf){
	uint32_t code = getKeyCode();
	if(code == 0xe){ // 退格符
		// TODO: 要求只能退格用户键盘输入的字符串，且最多退到当行行首
    if(bufferTail > bufferHead && keyBuffer[bufferTail] != '\n')
    {
      int currentHead = bufferTail - 1;
			while (currentHead > bufferHead && keyBuffer[currentHead - 1] != '\n')
      {
        currentHead--;
      }
			if(displayCol > 0)
      {
        displayCol--;
      }
			keyBuffer[bufferTail] = '\0';
      bufferTail--;
	    int sel = USEL(SEG_UDATA);
	    char character = 0;
	    uint16_t data = 0;
	    int pos = 0;
	    asm volatile("movw %0, %%es"::"m"(sel));
	    for (int i = 0; i < bufferTail - currentHead; i++) {
		    asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(keyBuffer+currentHead+i));
		    data = character | (0x0c << 8);
		    pos = (80 * displayRow + displayCol) * 2;
		    asm volatile("movw %0, (%1)"::"r"(data), "r"(pos + 0xb8000));
	    }
    }
	}else if(code == 0x1c){ // 回车符
		// TODO: 处理回车情况
    displayCol = 0;
		displayRow ++;
		keyBuffer[bufferTail] = '\n';
    bufferTail++;
	}else if(code < 0x81){ // 正常字符
		// TODO: 注意输入的大小写的实现、不可打印字符的处理
    char ch = getChar(code);
		if (ch >= 0x20) {
			putChar(ch);
			keyBuffer[bufferTail] = ch;
      bufferTail++;
			int sel = USEL(SEG_UDATA);
			char character = ch;
			uint16_t data = 0;
			int pos = 0;
			asm volatile("movw %0, %%es"::"m"(sel));
			data = character | (0x0c << 8);
			pos = (80 * displayRow + displayCol) * 2;
			asm volatile("movw %0, (%1)"::"r"(data), "r"(pos + 0xb8000));	
			displayCol ++;
			if (displayCol >= 80) {
				displayCol = 0;
				displayRow ++;
			}
			while (displayRow >= 25) {
				scrollScreen();
				displayRow --;
				displayCol = 0;
			}	
		}
	}
	updateCursor(displayRow, displayCol);
}

void syscallHandle(struct TrapFrame *tf) {
	switch(tf->eax) { // syscall number
		case 0:
			syscallWrite(tf);
			break; // for SYS_WRITE
		case 1:
			syscallRead(tf);
			break; // for SYS_READ
		default:break;
	}
}

void syscallWrite(struct TrapFrame *tf) {
	switch(tf->ecx) { // file descriptor
		case 0:
			syscallPrint(tf);
			break; // for STD_OUT
		default:break;
	}
}

void syscallPrint(struct TrapFrame *tf) {
	int sel = USEL(SEG_UDATA); //TODO: segment selector for user data, need further modification
	char *str = (char*)tf->edx;
	int size = tf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es"::"m"(sel));
	for (i = 0; i < size; i++) {
		asm volatile("movb %%es:(%1), %0":"=r"(character):"r"(str+i));
		// TODO: 完成光标的维护和打印到显存
    if(character == '\n')
    {
      displayCol = 0;
      displayRow++;
      if (displayRow >= 25) {
			  scrollScreen();
			  displayRow = 24;
			  displayCol = 0;
      } 
    }
    else
    {
      data = character | (0x0c << 8); 
      pos = (80 * displayRow + displayCol) * 2; 
      asm volatile("movw %0, (%1)"::"r"(data),"r"(pos+0xb8000));
      displayCol++;
    }
    if(displayCol >= 80)
    {
      displayCol = 0;
      displayRow++;
      if (displayRow >= 25) {
			  scrollScreen();
			  displayRow = 24;
			  displayCol = 0;
      }
    }
	}
	
	updateCursor(displayRow, displayCol);
}

void syscallRead(struct TrapFrame *tf){
	switch(tf->ecx){ //file descriptor
		case 0:
			syscallGetChar(tf);
			break; // for STD_IN
		case 1:
			syscallGetStr(tf);
			break; // for STD_STR
		default:break;
	}
}

void syscallGetChar(struct TrapFrame *tf){
	// TODO: 自由实现
  int flag = 0;
  if(keyBuffer[bufferTail - 1] == '\n')
  {
    flag = 1;
  }
	while (bufferTail > bufferHead && keyBuffer[bufferTail-1] == '\n')
  {
    bufferTail--;
    keyBuffer[bufferTail] = '\0';
  }
	if (bufferTail > bufferHead && flag == 1)
  { 
    tf->eax = keyBuffer[bufferHead];
    bufferHead++;
  }
	else
  {
    tf->eax = 0;
  }
}

void syscallGetStr(struct TrapFrame *tf){
	// TODO: 自由实现
  int flag = 0;
	int sel = USEL(SEG_UDATA);
	asm volatile("movw %0, %%es"::"m"(sel));
	if (keyBuffer[bufferTail - 1] == '\n')
  {
    flag = 1;
  }
	while (bufferTail > bufferHead && keyBuffer[bufferTail-1] == '\n')
  {
    bufferTail--;
    keyBuffer[bufferTail] = '\0';
  }
	if (flag == 0 && bufferTail - bufferHead < tf->ebx)
  {
    tf->eax = 0;
  }
	else 
  {
    //参考syscall.c中的dec2Str函数
		char str[256];
	  int count = 0, i = 0;
    int decimal = bufferTail - bufferHead;
    int temp;
    int number[16];
    // 处理负数
    if (decimal < 0) {
      str[count++] = '-';
      if (count == 256) 
      {
        putChar('\n');
        count = 0;
      }
      temp = decimal / 10;
      number[i++] = temp * 10 - decimal;
      decimal = temp;
      while (decimal != 0)
      {
        temp = decimal / 10;
        number[i++] = temp * 10 - decimal;
        decimal = temp;
      }
    } 
    else 
    {
      temp = decimal / 10;
      number[i++] = decimal - temp * 10;
      decimal = temp;
      while (decimal != 0) 
      {
        temp = decimal / 10;
        number[i++] = decimal - temp * 10;
        decimal = temp;
      }
    }
    // 将数字转换为字符串
    while (i != 0) 
    {
      str[count++] = number[i - 1] + '0';
      if (count == 256) 
      {
        putChar('\n');
        count = 0;
      }
      i--;
    }
    str[count] = '\0'; 
    // 输出字符串长度
    while (str[i] != '\0') putChar(str[i++]);
    putChar('\n');
		for (int i = 0; i < tf->ebx && i < bufferTail-bufferHead; i++) {
			asm volatile("movb %1, %%es:(%0)"::"r"(tf->edx+i), "r"(keyBuffer[bufferHead+i]));
		}
		tf->eax = 1;
	}
}