#include "Server.h"


int main()
{
#ifdef NETWORK_SERVER
	Server s;
	s.Run();
#endif

	return 0;
}