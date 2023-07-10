#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <stdio.h>

#include <math.h>
#include <vita2d.h>

#define sp 3

#define jf -850

#define cl 12
#define gl 539
#define rb 960
#define lb 0

#define ld .85f
#define rd .85f
//PSP2_MODULE_INFO(0, 0, "HelloWorld");df


struct rect {
	int x;
	int y;
	int width;
	int height;
};
 
/*
	Just checks collisions between player and other rectangle
*/
int isIntersecting(struct rect rect1, struct rect rect2) {
    return rect1.x < rect2.x + rect2.width &&
           rect1.x + rect1.width > rect2.x &&
           rect1.y < rect2.y + rect2.height &&
           rect1.y + rect1.height > rect2.y;
}

/*

*/
void calculateCollisionDisplacement(struct rect* rect1, struct rect rect2, int *gr, float *jSpeed) {
    int dx = 0, dy = 0;

    // Calculate the overlap on each side of the rectangles
    int overlapLeft = rect2.x + rect2.width - rect1->x;
    int overlapRight = rect1->x + rect1->width - rect2.x;
    int overlapTop = rect2.y + rect2.height - rect1->y;
    int overlapBottom = rect1->y + rect1->height - rect2.y;


    if (overlapTop < overlapBottom) {
        rect1->y = rect2.y + rect2.height;
		*(jSpeed) = -*(jSpeed);
    } else if (overlapTop > overlapBottom) {
		
		//dy = -overlapTop;
		rect1->y = rect2.y;
		*gr =1;
	} 
} 

int main() {
	vita2d_pgf *pgf;
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITAL);
	SceCtrlData ctrl;

	int x = 700;
	int y = 30;
	
	struct rect pl = {x,y,60,15};
	
	struct rect fl[3] = {{960/4, 544/2,120,15},{(960/4)* 3, 544/2,200,60}};

	uint64_t prevTime = sceKernelGetProcessTimeWide();
    float deltaTime = 0.0f;

	int jumpCount = 0;
	float jumpForce = jf;
    float jumpSpeed=jumpForce;

	int ground = 0;
	float gravity = 500.0f;
	
	char scoreText[100];
	char structText[50];
	int col = 0; 
	int jumpButtonPressed = 0;
	int forcePressed= 0;
 
	float acceleration = 400.0f;
    float velocity = 0.0f;

	float platformVel = 100.0f;

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

		if (pl.y == gl || ground == 1) {
            ground = 1;
            jumpCount = 0;
        } else if (pl.y < gl && ground ==0) {
            ground = 0;
            pl.y += gravity * deltaTime;
        }

		//Controls setup
		if (ctrl.buttons & SCE_CTRL_LEFT || ctrl.buttons & SCE_CTRL_RIGHT) {
            acceleration = 400.0f;
        } else {
            acceleration = 0.0f;
			velocity = 0.0f;
        }

 		if(ctrl.buttons & SCE_CTRL_LEFT) {velocity -= acceleration*deltaTime;}
		if(ctrl.buttons & SCE_CTRL_RIGHT) {velocity += acceleration*deltaTime;}

		if (velocity < 0) {
			velocity -= acceleration * deltaTime;
			velocity *= ld; // Apply damping factor for left movement
		}

		if (velocity > 0) {
			velocity += acceleration * deltaTime;
			velocity *= rd; // Apply damping factor for right movement
		}
		pl.x += sp * velocity * deltaTime;

		//Jump button
		if (ctrl.buttons & SCE_CTRL_CROSS && !jumpButtonPressed && jumpCount < 4) {
            jumpSpeed = jumpForce; // Initial jump velocity
			ground = 0;
            jumpCount++;
			jumpButtonPressed = 1;
        }
		if (!(ctrl.buttons & SCE_CTRL_CROSS)) {
            jumpButtonPressed = 0;
        }

		//Modifies jump force
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

		if (!(ctrl.buttons & SCE_CTRL_RTRIGGER) && !(ctrl.buttons & SCE_CTRL_LTRIGGER))
		{
			forcePressed = 0;
		} 
		if (!ground) {
            pl.y += jumpSpeed * deltaTime; // Apply jump velocity
            jumpSpeed += gravity * deltaTime; // Apply gravity
        } else {
			jumpCount = 0;
		}
		
		//prevents player from going off screen
		if(pl.x+pl.width > rb) {pl.x = rb-pl.width;}
		if(pl.x < lb)  {pl.x = lb;}
		if(pl.y > gl) {pl.y = gl;}
		if(pl.y < 12)  {pl.y = 12;}
		
		if (fl[0].x+fl[0].width == fl[1].x || fl[0].x == lb) {
			platformVel = -(platformVel);
		}
		fl[0].x += platformVel * deltaTime;
	
		vita2d_pgf_draw_text(pgf, x, y, RGBA8(0, 255, 0, 255), 1.0f, "Hello, World!");
 
		//Debug Text rendering, updated to reflect changing values
        sprintf(scoreText, "X: %d, Y: %d, ground: %d, jumpcount: %d, jumpforce: %.0f", fl[0].x,fl[0].y,ground, jumpCount, jumpForce);
		vita2d_pgf_draw_text(pgf, 0, 15, RGBA8(0, 255, 0, 255), 1.0f, scoreText);

		sprintf(structText, "sx: %d sy: %d velocity: %.4f collision: %d", pl.x, pl.y, velocity, col);
		vita2d_pgf_draw_text(pgf, 0, 30, RGBA8(0, 255, 0, 255), 1.0f, structText);
		
		//vita2d_draw_rectangle(x,y-15,120,15,RGBA8(0, 255, 0, 255));
		
		if (isIntersecting(pl, fl[0])) {
            // Adjust the position to separate the rectangles 
		 	calculateCollisionDisplacement(&pl, fl[0], &ground, &jumpSpeed);
			pl.x += platformVel * deltaTime;
			col = 1;
		} else if (isIntersecting(pl, fl[1])) {
            // Adjust the position to separate the rectangles 
			calculateCollisionDisplacement(&pl, fl[1], &ground, &jumpSpeed);
			col = 1;
		} else {
			col = 0;
			ground = 0;
		}

		//Rectangles get drawn 
		vita2d_draw_rectangle(pl.x, pl.y-15,pl.width,pl.height,RGBA8(0, 255, 255, 255));
		
		
		for (int i=0; i<2; i++)
		{
			vita2d_draw_rectangle(fl[i].x, fl[i].y,fl[i].width,fl[i].height,RGBA8(255, 255, 0, 255));
		}
		
		//vita2d_draw_rectangle(fl[0].x, fl[0].y,fl[0].width,fl[0].height,RGBA8(255, 255, 0, 255));
		//vita2d_draw_rectangle(fl[1].x, fl[1].y,fl[1].width,fl[1].height,RGBA8(255, 255, 0, 255));
		
		vita2d_end_drawing();
		vita2d_swap_buffers();
	}
	
	vita2d_fini();
	vita2d_free_pgf(pgf);
	sceKernelExitProcess(0);
	return 0;
}