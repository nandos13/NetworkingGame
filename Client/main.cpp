#include "Client.h"

int main() {
	
	auto app = new Client();
	app->run("AIE", 640, 480, false);
	delete app;

	return 0;
}