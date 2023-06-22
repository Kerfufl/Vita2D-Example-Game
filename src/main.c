#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <stdio.h>

#include <vita2d.h>


#define jf -750
//PSP2_MODULE_INFO(0, 0, "HelloWorld");


struct rect {
	int x;
	int y;
	int width;
	int height;
};

int main() {
	vita2d_pgf *pgf;
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITAL);
	SceCtrlData ctrl;

	int x = 700;
	int y = 30;
	
	struct rect pl = {x,y,120,10};
	uint64_t prevTime = sceKernelGetProcessTimeWide();
    float deltaTime = 0.0f;
	int jumpCount = 0;
	float jumpForce = jf;
    float jumpSpeed=jumpForce;
	int ground = 1;
	float gravity = 500.0f;

	int jumpButtonPressed = 0;
	int forcePressed= 0;

	vita2d_init();
	vita2d_set_clear_color(RGBA8(0, 0, 0, 255));

	pgf = vita2d_load_default_pgf();
	
	while(1){
		vita2d_start_drawing();
		vita2d_clear_screen();

		uint64_t currentTime = sceKernelGetProcessTimeWide();
        deltaTime = (float)(currentTime - prevTime) / 1000000.0f; // Convert to seconds

		prevTime = currentTime;

		sceCtrlPeekBufferPositive(0, &ctrl, 1);

		if (y == 539) {
            ground = 1;
            jumpCount = 0;
        } else if (y < 539) {
            ground = 0;
            y += gravity * deltaTime;
        }
		//bruh
		//y += 1500*5*deltaTime;

 		if(ctrl.buttons & SCE_CTRL_LEFT) {x -= 200*deltaTime;}
		if(ctrl.buttons & SCE_CTRL_RIGHT) {x += 200*deltaTime;}
		
		// if(ctrl.buttons & SCE_CTRL_UP) {y -= 5;}
		// if(ctrl.buttons & SCE_CTRL_DOWN) {y+= 5;}

		if (ctrl.buttons & SCE_CTRL_CROSS && !jumpButtonPressed && jumpCount < 4) {
            jumpSpeed = jumpForce; // Initial jump velocity
			ground = 0;
            jumpCount++;
			jumpButtonPressed = 1;
        }

		if (ctrl.buttons & SCE_CTRL_RTRIGGER  && !forcePressed)
		{
			jumpForce -= 50;
			forcePressed= 1;
		}
		if (ctrl.buttons & SCE_CTRL_LTRIGGER  && !forcePressed)
		{
			jumpForce += 50;
			forcePressed= 1;
		}

		if (!(ctrl.buttons & SCE_CTRL_CROSS)) {
            jumpButtonPressed = 0;
        }

		if (!(ctrl.buttons & SCE_CTRL_RTRIGGER) && !(ctrl.buttons & SCE_CTRL_LTRIGGER))
		{
			forcePressed = 0;
		}
		if (!ground) {
            y += jumpSpeed * deltaTime; // Apply jump velocity
            jumpSpeed += gravity * deltaTime; // Apply gravity
        }

		
		if(x > 840) {x = 840;}
		if(x < 10)  {x = 10;}
		if(y > 539) {y = 539;}
		if(y < 12)  {y = 12;}


		vita2d_pgf_draw_text(pgf, x, y, RGBA8(0, 255, 0, 255), 1.0f, "Hello, World!");
		char scoreText[20];

        sprintf(scoreText, "X: %d, Y: %d, ground: %d, jumpcount: %d, jumpforce: %.0f", x,y,ground, jumpCount, jumpForce);
		vita2d_pgf_draw_text(pgf, 0, 15, RGBA8(0, 255, 0, 255), 1.0f, scoreText);
		
		vita2d_draw_rectangle(x,y-15,120,15,RGBA8(0, 255, 0, 255));
		
		
		vita2d_end_drawing();
		vita2d_swap_buffers();
	}
	
	vita2d_fini();
	vita2d_free_pgf(pgf);
	sceKernelExitProcess(0);
	return 0;
}