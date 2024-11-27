
void Delay(int time) {
	while(--time > 0);
}

int flag;
int main() {

	for(;;) {
		flag = 0;
		Delay(200);
		flag = 1;
		Delay(200);
	}
	return 0;
}