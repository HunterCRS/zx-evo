#include "std.h"
#include "emul.h"
#include "vars.h"
#include "debug.h"
#include "dbgtrace.h"
#include "util.h"
#include "consts.h"

/*
	 dialogs design

�����������������������Ŀ
�Read from TR-DOS file  �
�����������������������Ĵ
�drive: A               �
�file:  12345678 C      �
�start: E000 end: FFFF  �
�������������������������

�����������������������Ŀ
�Read from TR-DOS sector�
�����������������������Ĵ
�drive: A               �
�trk (00-9F): 00        �
�sec (00-0F): 08        �
�start: E000 end: FFFF  �
�������������������������

�����������������������Ŀ
�Read RAW sectors       �
�����������������������Ĵ
�drive: A               �
�cyl (00-4F): 00 side: 0�
�sec (00-0F): 08 num: 01�
�start: E000            �
�������������������������

�����������������������Ŀ
�Read from host file    �
�����������������������Ĵ
�file: 12345678.bin     �
�start: C000 end: FFFF  �
�������������������������

*/



u8 *memdata;


