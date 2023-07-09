#include <psp2/display.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/ctrl.h>
#include <stdio.h>

#include <math.h>
#include <vita2d.h>

#define sp 3

#define jf -850

#define gl 539
#define rb 840
#define lb 10

#define ld .85f
#define rd .85f
//PSP2_MODULE_INFO(0, 0, "HelloWorld");df


struct rect {
	int x;
	int y;
	int width;
	int height;
};

int isIntersecting(struct rect rect1, struct rect rect2) {
    return rect1.x < rect2.x + rect2.width &&
           rect1.x + rect1.width > rect2.x &&
           rect1.y < rect2.y + rect2.height &&
           rect1.y + rect1.height > rect2.y;
}

void calculateCollisionDisplacement(struct rect* rect1, struct rect rect2, int *gr) {
    int dx = 0, dy = 0;

    // Calculate the overlap on each side of the rectangles
    int overlapLeft = rect2.x + rect2.width - rect1->x;
    int overlapRight = rect1->x + rect1->width - rect2.x;
    int overlapTop = rect2.y + rect2.height - rect1->y;
    int overlapBottom = rect1->y + rect1->height - rect2.y;

    // Determine the direction of collision and the minimum displacement needed
    if (overlapLeft < overlapRight) {
        dx = -overlapLeft;
    } else if ( overlapRight < overlapLeft) {
        dx = overlapRight;
    }

    if (overlapTop < overlapBottom) {
        dy = -overlapTop;
		rect1->y = rect2.y;
		*gr =1;
    } else {
        //dy = overlapBottom;
		//rect1->y = rect2.y+rect2.height;
		//*gr=0;
    }

    // Adjust the position of rect1 to push it away from rect2
    //rect1->x = 2 *dx;
    //rect1->y += 1.5*dy;
} 
 
int main() {
	vita2d_pgf *pgf;
	sceCtrlSetSamplingMode(SCE_CTRL_MODE_DIGITAL);
	SceCtrlData ctrl;

	int x = 700;
	int y = 30;
	
	struct rect pl = {x,y,60,15};
	struct rect fl = {960/2, 544/2,120,15};
	uint64_t prevTime = sceKernelGetProcessTimeWide();
    float deltaTime = 0.0f;
	int jumpCount = 0;
	float jumpForce = jf;
    float jumpSpeed=jumpForce;
	int ground = 0;
	float gravity = 500.0f;
	
	char scoreText[80];
	char structText[40];
	int col = 0; 
	int jumpButtonPressed = 0;
	int forcePressed= 0;

	float acceleration = 400.0f;
    float velocity = 0.0f;

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
		//bruh
		//y += 1500*5*deltaTime;

		//Controls setup
		if (ctrl.buttons & SCE_CTRL_LEFT || ctrl.buttons & SCE_CTRL_RIGHT) {
            acceleration = 400.0f;
        } else {
            acceleration = 0.0f;
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
		//velocity = .4f * velocity;

		//such a pien, why why

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
            pl.y += jumpSpeed * deltaTime; // Apply jump velocity
            jumpSpeed += gravity * deltaTime; // Apply gravity
        } else {
			jumpCount = 0;
		}
		
		
		if(pl.x > rb) {pl.x = rb;}
		if(pl.x < lb)  {pl.x = lb;}
		if(pl.y > gl) {pl.y = gl;}
		if(pl.y < 12)  {pl.y = 12;}

		if (isIntersecting(pl, fl)) {
             // Adjust the position to separate the rectangles 
			 calculateCollisionDisplacement(&pl, fl, &ground);
			col = 1;
		} else {
			col = 0;
			ground = 0;
		}
        
		vita2d_pgf_draw_text(pgf, x, y, RGBA8(0, 255, 0, 255), 1.0f, "Hello, World!");
		

		//Debug Text rendering, updated to reflect changing values
        sprintf(scoreText, "X: %d, Y: %d, ground: %d, jumpcount: %d, jumpforce: %.0f", fl.x,fl.y,ground, jumpCount, jumpForce);
		vita2d_pgf_draw_text(pgf, 0, 15, RGBA8(0, 255, 0, 255), 1.0f, scoreText);

		sprintf(structText, "sx: %d sy: %d velocity: %.4f collision: %d", pl.x, pl.y, velocity, col);
		vita2d_pgf_draw_text(pgf, 0, 30, RGBA8(0, 255, 0, 255), 1.0f, structText);
		
		//vita2d_draw_rectangle(x,y-15,120,15,RGBA8(0, 255, 0, 255));
		

		//Rectangles get drawn
		vita2d_draw_rectangle(pl.x, pl.y-15,pl.width,pl.height,RGBA8(0, 255, 255, 255));
		
		vita2d_draw_rectangle(fl.x, fl.y,fl.width,fl.height,RGBA8(255, 255, 0, 255));
		
		vita2d_end_drawing();
		vita2d_swap_buffers();
	} 
	
	vita2d_fini();
	vita2d_free_pgf(pgf);
	sceKernelExitProcess(0);
	return 0;
}