#include "font.h"

int Show_Char()
{
	char *String = "cxdtest";
	Init_Font(); //????????	

	//??????¦Ë?????????
	Clean_Area(0,  
               0,  
			   800, 
			   480,
			   0xFFFFFFFF); 	
	//?????????????
	Display_characterX(100,          
                       100,          
					   String,   
					   0xFF ,     
					   4); 		
	UnInit_Font(); 
	return 0;
}