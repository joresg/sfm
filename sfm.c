#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct ls_rets{
	char ** str;
	int len;
}ls_ret;

int is_dir(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

ls_ret *ls(char *dir_name){
	ls_ret *lr = (ls_ret *)malloc(sizeof(ls_ret));
	char **ls_dir;
	int num_dir = 0;
	ls_dir = (char **)malloc(sizeof(char *));
	DIR *d;
	struct dirent *dir;
	d = opendir(dir_name);
	if(d){
		while((dir = readdir(d)) != NULL){
			if(strcmp(dir->d_name,".") && strcmp(dir->d_name,"..")){
				ls_dir[num_dir] = (char *)malloc(100*sizeof(char));
				strcpy(ls_dir[num_dir], dir->d_name);
				num_dir++;
				ls_dir = (char **)realloc(ls_dir, (num_dir+1)*sizeof(char *));	
			}
		}
	closedir(d);
	}
	lr->str = ls_dir;
	lr->len = num_dir;
	
	return lr;
}
void ls_free(ls_ret *lr){
	for(int i=0;i<lr->len;i++){
		free(lr->str[i]);
	}
	free(lr);
}

int find_nth_last_char(char *str, char target, int pos){
	//printf("target: %c\n",target);
	printf("string %s iscem %d %c\n", str, pos, target);
	int x=strlen(str);
	//while(str[x]!='\0'){
	while(x>=0){
		if(str[x] == target){
			if(pos == 1){
				return x;
			}else{
				pos--;
			}
		}
		x--;
	}
	return -1;
}
char *substr(char *str, int start, int finish){
	printf("length %d\n",finish-start);
	int x = 0;
	char *ret = (char *)malloc((finish-start)*sizeof(char));
	for(int i=start;i<finish;i++){
		ret[x] = str[i];
		x++;
	}
	return ret;
}

int main(int argc, char **argv){

	Display *dpy;
	Window root, win;
	int screen;
	GC gc;
	XColor color1;
	XSetWindowAttributes swa;
	XWindowAttributes wa;
	int winx = 0, winy = 0;
	unsigned int winw = 1280, winh = 720;

	dpy = XOpenDisplay(NULL);
	screen = DefaultScreen(dpy);
	root = RootWindow(dpy, screen);

	Colormap cmap = DefaultColormap(dpy, screen);
	XParseColor(dpy, cmap, "#198c8f", &color1);
	XAllocColor(dpy, cmap, &color1);

	//set window attributes
	swa.background_pixel = BlackPixel(dpy, screen);
	swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | VisibilityChangeMask;

	//create window
	win = XCreateWindow(dpy, root, winx, winy, winw, winh, 2, CopyFromParent, CopyFromParent, CopyFromParent, CWBackPixel | CWEventMask, &swa);
	XStoreName(dpy, win, "sfm");
	XGetWindowAttributes(dpy, win, &wa);

	//load font which you will use
	//with the given CG
	int npaths_return;
	printf("%s\n","FUCK");
	char **gfp = XGetFontPath(dpy, &npaths_return); 
	printf("num of paths: %d\n",npaths_return);
	for(int i=0;i<npaths_return;i++){
		printf("%s\n",gfp[i]);
	}
	printf("%s\n","OK");
	/*
	char **font_dirs = (char **)malloc(sizeof(char *));
	font_dirs[0] = (char *)malloc(40*sizeof(char));
	strcpy(font_dirs[0], "/usr/share/fonts/Ubuntu");
	XSetFontPath(dpy, font_dirs, 4);
	*/

	Font font = XLoadFont(dpy, "-adobe-utopia-regular-r-normal--33-240-100-100-p-180-iso8859-14");	
	
	
	//set GC attributes
	XGCValues sgc;
	sgc.font = font;
	gc = XCreateGC(dpy, win, GCFont, &sgc);

	//map window (show it) and select it as input
	XMapWindow(dpy, win);
	XSelectInput(dpy, win, ExposureMask | KeyPressMask | KeyReleaseMask | VisibilityChangeMask);

	//handle events
	XEvent event;

	char *dir = (char *)malloc(500*sizeof(char));
	strcpy(dir, "/home/joresg/");

	int choice_dir = 0;
	int content_start = 0, num_of_displayed_dirs = wa.height/40-2;
	while(!XNextEvent(dpy, &event)){
		ls_ret *subdirs = ls(dir);
		char **subdirs_name = subdirs->str;
		printf("choice dir: %d\n", choice_dir);
		printf("CURRENTLY SELECTED: %s\n", subdirs_name[choice_dir]);
		//first draw background
		XSetForeground(dpy, gc, BlackPixel(dpy, screen));
		XFillRectangle(dpy, win, gc, 0, 0, wa.width, wa.height);
		//draw text
		XSetForeground(dpy, gc, WhitePixel(dpy, screen));
		//get all subdirs
		//izpisi vse directory-je
		int textx = 10, texty = 30;
		if(subdirs->len == 0){
			XDrawString(dpy, win, gc, textx, texty, "EMPTY", 5);
		}else{
			//for(int i=content_start;i<content_start+num_of_displayed_dirs;i++){
			for(int i=content_start;i<subdirs->len && i<content_start+num_of_displayed_dirs;i++){
				if(i == choice_dir){
					printf("i: %d cd: %d\n",i,choice_dir);
					XSetForeground(dpy, gc, color1.pixel);
					XFillRectangle(dpy, win, gc, 0, (choice_dir)*40, wa.width, 40);
					XSetForeground(dpy, gc, BlackPixel(dpy, screen));
					XDrawString(dpy, win, gc, textx, texty, subdirs_name[i], strlen(subdirs_name[i]));
				}else{
					XSetForeground(dpy, gc, WhitePixel(dpy, screen));
					XDrawString(dpy, win, gc, textx, texty, subdirs_name[i], strlen(subdirs_name[i]));
				}
				texty+=40;
			}
		}
		printf("is %s dir? %d\n",dir, is_dir(dir));
		switch(event.type){
			case Expose:
				printf("%s\n","expose");
				XGetWindowAttributes(dpy, win, &wa);
				break;
			case KeyPress:
				switch(event.xkey.keycode){
					case 22:
						strcpy(dir, "/home/joresg/");
						choice_dir = 0;
						break;
					case 43:
						printf("%s\n","goin to parent dir");
						if(strcmp(dir,"/")){
							printf("nisem se v root, LP\n");
							int substrend = find_nth_last_char(dir, '/', 2);
							if(!substrend){
								strcpy(dir, "/");
							}else{
								char *dir_substr = substr(dir, 0, substrend+1);
								strcpy(dir, dir_substr);
								free(dir_substr);
							}
						}
						printf("NOV PARENT DIR: %s\n", dir);
						choice_dir = 0;
						content_start = 0;
						break;
					case 44:
						if(choice_dir < subdirs->len-1){
							choice_dir++;
							if(choice_dir == content_start+num_of_displayed_dirs){
								content_start++;
							}
						}
						break;
					case 45:
						if(choice_dir>0){
							choice_dir--;
							if(choice_dir<content_start){
								content_start--;
							}
						}
						break;
					case 46:
						printf("goin in hot\n");
						printf("dir....%s\nsubidr.....%s\n",dir, subdirs_name[choice_dir]);
						strcat(dir,subdirs_name[choice_dir]);
						if(is_dir(dir)){
							strcat(dir,"/");
							choice_dir = 0;
							content_start = 0;
							printf("NOVIDIR: %s\n",dir);
						}
						else{
							//odpri file
							//poglej se ce se odpre v terminalu
							//ce ja spawnaj prej se terminal in
							//odpri program v njemu
							printf("odpiram file");
							char *cmd = (char *)malloc((5+strlen(dir))*sizeof(char));
							strcpy(cmd, "xdg-open ");
							strcat(cmd, dir);
							strcat(cmd, " &");
							printf("cmd: %s\n",cmd);
							int status = system(cmd);
							free(cmd);
						}
						printf("pridem sploh iz tega sranja?");
						break;
					default:
						printf("%s\n","kpress");
						printf("keycode: %d\n", event.xkey.keycode);
						//KeySym temp = XKeycodeToKeysym(ekran, event.xkey.keycode, int index); dfuk je index?
						KeySym temp = XKeycodeToKeysym(dpy, event.xkey.keycode, 0);
						char *wat = XKeysymToString(temp);
						printf("keysym %s\n", wat);
						if(!strcmp(wat, "space")){
							wat = " ";
						}
						break;
				}
				break;
			default:
				break;
		}
		printf("FREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");
		ls_free(subdirs);
	}
	return(0);
}
