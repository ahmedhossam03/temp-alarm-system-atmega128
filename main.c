#include "temp_alarm_system.h"

int main()
{
	tas_init();
	
	while(1)
	{
		tas_run();
		
	}
}