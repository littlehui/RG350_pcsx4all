
#include <time.h>
#include "cjs_font.h"
#include <SDL/SDL.h>




//extern SDL_Surface *mScreen;
//extern SCheatData Cheat;
//SDL

TTF_Font *sdl_ttf_font = NULL;
SDL_Surface *g_font = NULL;
SDL_Surface *gui_screen = NULL;
//Screen dimension constants
const int FONT_SIZE = 14;

SDL_Surface *g_bg;


int InitFont()
{
	// Load picture
/*	if ( g_bg == NULL ){
		g_bg = SDL_LoadBMP("backdrop.bmp");
		if ( g_bg == NULL ){
			return -1;
		}
	}*/
	//SDL_ttf
	//预处理
	if( sdl_ttf_font == NULL ){
		TTF_Init();
		//sdl_ttf_font = TTF_OpenFont("/usr/share/gmenu2x/skins/Default/fonts/SourceHanSans-Regular-04.ttf", FONT_SIZE);
        sdl_ttf_font = TTF_OpenFont("SourceHanSans-Regular-04.ttf", FONT_SIZE);
        if(sdl_ttf_font == NULL){
			return -1;
		}
	}
	return 0;
}

void KillFont()
{
	SDL_FreeSurface(g_bg);
	//SDL_ttf
	TTF_CloseFont( sdl_ttf_font );
	TTF_Quit();
	//SDL_Quit();
}



void draw_bg(SDL_Surface *bg)
{
	if(bg)
		SDL_BlitSurface(bg, NULL, gui_screen, NULL);
		//SDL_BlitSurface(gui_screen, NULL, bg, NULL);
	else
		SDL_FillRect(gui_screen, NULL, (1<<11) | (8<<5) | 10);
}



//矩形填充，用于着重显示选择项
void drawrect(Uint8 R, Uint8 G, Uint8 B,int x,int y,int width,int heigth)
{
    //str_log("before  color SDL_MapRGB");

    Uint32 color = SDL_MapRGB(gui_screen->format, R, G, B);
    //str_log("after  color SDL_MapRGB");
    SDL_Rect rect = { x, y, width, heigth };
    //str_log("after  define SDL_Rect");
    SDL_FillRect(gui_screen, &rect, color);
    //str_log("after  SDL_FillRect");

}

void DrawText3(const char *textmsg, int x, int y)
{
	if( textmsg == NULL ) return;
	SDL_Color textColor={255, 255, 255};//设置颜色
	SDL_Surface *message=NULL;
	message=TTF_RenderUTF8_Blended(sdl_ttf_font,textmsg, textColor );//加在成中文
	//message=TTF_RenderUTF8_Solid(sdl_ttf_font,textmsg, textColor );//加在成中文
	SDL_Rect dect;
	dect.x=x;
	dect.y=y;
	dect.h=0;
	dect.w=0;
	SDL_BlitSurface(message, NULL, gui_screen, &dect);
	SDL_FreeSurface(message);
}

void DrawText(const char *textmsg, int x, int y, SDL_Surface *surface)
{
    if( textmsg == NULL ) return;
    SDL_Color textColor={255, 255, 255};//设置颜色
    SDL_Surface *message=NULL;
    message=TTF_RenderUTF8_Blended(sdl_ttf_font,textmsg, textColor );//加在成中文
    //message=TTF_RenderUTF8_Solid(sdl_ttf_font,textmsg, textColor );//加在成中文
    SDL_Rect dect;
    dect.x=x;
    dect.y=y;
    dect.h=0;
    dect.w=0;
    SDL_BlitSurface(message, NULL, surface, &dect);
    SDL_FreeSurface(message);
    SDL_Flip(surface);
    if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);
}

//
//log
void str_log(char *info)
{
	//获取时间
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);//获取Unix时间戳。
	timeinfo = localtime (&rawtime);;//转为时间结构。
	char now_time[128];
	strftime (now_time,sizeof(now_time),"%Y-%m-%d,%H:%M:%S - :",timeinfo);

	//log记录
	//char *fn=NULL;
	//fn=strdup(FCEU_MakeFName(FCEUMKF_CHEAT,0,0).c_str());
	//fn = "./debug.chj";
	//char *file_name=stringcat(log,"_debug.log");
	//free(fn);

	char log[] = "/media/sdcard/apps/sfc.log";
	FILE *fp_log = fopen (log,"a+");
	//free(file_name);
	//if(!fp_log) return 1;
	fputs(now_time, fp_log);
	fputs(info, fp_log);
	fputs(":", fp_log);
	fputs(log, fp_log);
	fputs("\n", fp_log);
	fflush(fp_log);
	fclose(fp_log);
}