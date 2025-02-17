#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <ctype.h>
#include <X11/Xft/Xft.h>


#define POSITIVE(n) ((n) < 0 ? 0 - (n) : (n))
#define SPECIAL_CHAR(c) ((c) == ' ' || (c) == '`' || (c) == '~' || (c) == '!' \
		|| (c) == '@' || (c) == '#' || (c) == '$' || (c) == '%' \
		|| (c) == '^' || (c) == '&' || (c) == '*' || (c) == '(' \
		|| (c) == ')' || (c) == '-' || (c) == '_' || (c) == '=' \
		|| (c) == '+' || (c) == '{' || (c) == '}' || (c) == '|' \
		|| (c) == ';' || (c) == ':' || (c) == '"' || (c) == ',' \
		|| (c) == '<' || (c) == '.' || (c) == '>' || (c) == '?' \
		|| (c) == '\'' || (c) == '\\' ? 1 : 0)

typedef struct ls_rets{
	char ** str;
	int len;
}ls_ret;

typedef struct _clr_scheme{
	XftColor background;
	XftColor foreground;
	XftColor highlight;
}color_scheme;

int is_dir(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

char *str2lower(char *str){
	char *ret = (char *)malloc((strlen(str)+1)*sizeof(char));
	strcpy(ret, str);
	for(int i = 0; ret[i]; i++){
  		ret[i] = tolower(ret[i]);
	}
	return ret;
}
void dir_sort(ls_ret *lr){
	int menjava = 1;
	int ret;
	char *tmp;
	while(menjava){
		menjava = 0;
		for(int i=0;i<lr->len-1;i++){
			char *tolwr1 = str2lower(lr->str[i]);
			char *tolwr2 = str2lower(lr->str[i+1]);
			//ret = strcmp(lr->str[i], lr->str[i+1]);
			ret = strcmp(tolwr1, tolwr2);
			free(tolwr1);
			free(tolwr2);
			if(ret>0){
				tmp = (char *)malloc((strlen(lr->str[i+1])+1)*sizeof(char));
				strcpy(tmp, lr->str[i+1]);
				strcat(tmp,"\0");
				free(lr->str[i+1]);
				lr->str[i+1] = lr->str[i];
				lr->str[i] = tmp;
				menjava = 1;
			}
		}
	}
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
			if(dir->d_name[0] != '.'){
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
	//sort alphabetically
	dir_sort(lr);
	
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
	int x = 0;
	char *ret = (char *)malloc((finish-start)*sizeof(char));
	for(int i=start;i<finish;i++){
		ret[x] = str[i];
		x++;
	}
	ret[x] = '\0';
	return ret;
}

void draw(Display *dpy, int screen, Window win, GC gc, XftDraw *d, XftFont *font, color_scheme *colors, XWindowAttributes wa, ls_ret *subdirs, int selected_dir, int content_start, char *base_dir){
	//get window attributes first
	XGetWindowAttributes(dpy, win, &wa);
	//draw background
	XftDrawRect(d, &colors->background , 0, 0, wa.width, wa.height);

	//get all files in dir
	char **subdirs_name = subdirs->str;
	int font_asc_dsc = font->ascent + font->descent;
	int textx = 10, texty = font->ascent, num_of_displayed_dirs = wa.height/font_asc_dsc;

	//dir is empty
	char *check_dir;
	char *tmp;
	if(subdirs->len == 0){
		XftDrawString8(d, &colors->foreground, font, textx, texty, (XftChar8 *)"EMPTY", 5);
	}else{
		for(int i=content_start;i<subdirs->len && i<content_start+num_of_displayed_dirs;i++){
			//highlight selected dir
			if(i == selected_dir){
				printf("i: %d cd: %d\n",i,selected_dir);
				XftDrawRect(d, &colors->highlight, 0, (selected_dir-content_start)*font_asc_dsc, wa.width, font_asc_dsc);
			}
			check_dir = (char *)malloc((strlen(base_dir)+strlen(subdirs_name[i])+1)*sizeof(char));
			strcpy(check_dir, base_dir);
			strcat(check_dir, subdirs_name[i]);
			tmp = (char *)malloc((strlen(subdirs_name[i])+5)*sizeof(char));
			if(is_dir(check_dir)){
				strcpy(tmp, "(d) ");
				strcat(tmp, subdirs_name[i]);
				XftDrawString8(d, &colors->foreground, font, textx, texty, (XftChar8 *)tmp, strlen(tmp));
			}else{
				strcpy(tmp, "(f) ");
				strcat(tmp, subdirs_name[i]);
				XftDrawString8(d, &colors->foreground, font, textx, texty, (XftChar8 *)tmp, strlen(tmp));
			}
			free(tmp);
			free(check_dir);
			texty+=font_asc_dsc;
		}
	}
}
int dir_empty(ls_ret *dirs){
	if(dirs->len == 0){
		return 1;
	}
	return 0;
}

char *handle_special_chars(char *str){
	int num_of_spec_chars = 0;
	for(int i=0;i<strlen(str);i++){
		if(SPECIAL_CHAR(str[i])){
			num_of_spec_chars++;
		}
	}
	if(num_of_spec_chars == 0){
		return str;
	}
	printf("num_of_spec_chars: %d\n", num_of_spec_chars);
	char *ret = (char *)malloc((strlen(str)+num_of_spec_chars+1)*sizeof(char));
	int x=0;
	for(int i=0;i<strlen(str);i++){
		//if(str[i] == ' '){
		if(SPECIAL_CHAR(str[i])){
			ret[x] = '\\';
			x++;
			ret[x] = str[i];
		}
		else{
			ret[x] = str[i];
		}
		x++;
	}
	ret[x]='\0';
	printf("ret: %s\n", ret);
	free(str);
	return ret;
}

int remove_file_dir(char *target){
		DIR *d;
		struct dirent *dir;
		d = opendir(target);
		int status;
		if(d){
			char *subdir_name;
			while((dir = readdir(d)) != NULL){
				if(POSITIVE(strcmp(dir->d_name,".")) && POSITIVE(strcmp(dir->d_name,".."))){
					subdir_name = (char *)malloc((strlen(target)+strlen(dir->d_name)+2)*sizeof(char));
					strcpy(subdir_name, target);
					strcat(subdir_name, "/");
					strcat(subdir_name, dir->d_name);
					if(is_dir(subdir_name)){
						remove_file_dir(subdir_name);
					}else{
						status = remove(subdir_name);
					}
					free(subdir_name);
				}
			}
			status = remove(target);
			closedir(d);
			return(status);
		}
		return remove(target);
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

		Bool supported_rtrn;
		Bool xkbar = XkbSetDetectableAutoRepeat(dpy, True, &supported_rtrn);

		Colormap cmap = DefaultColormap(dpy, screen);
		XParseColor(dpy, cmap, "#198c8f", &color1);
		XAllocColor(dpy, cmap, &color1);

		//const char * fontname = "Ubuntu Mono:size=18";
		const char * fontname = "IBM Plex Mono:size=14";
		//const char * fontname = "Source Code Pro:size=18";
		XftFont *font = XftFontOpenName(dpy, screen, fontname);
		//XftFont* font = XftFontOpen(dpy, DefaultScreen(dpy), XFT_FAMILY, XftTypeString, "ubuntu", XFT_SIZE, XftTypeDouble, 14.0, NULL);
		printf("ascent: %d, descent: %d, height: %d\n", font->ascent, font->descent, font->height);

		color_scheme *colors = (color_scheme *)malloc(sizeof(color_scheme));
		//Xft colors
		XRenderColor xrcolor1, xrcolor2, xrcolor3;
		/*
		xrcolor1.red = 65535;
		xrcolor1.green = 65535;
		xrcolor1.blue = 65535;
		*/
		xrcolor1.red = 223*257;
		xrcolor1.green = 172*257;
		xrcolor1.blue = 160*257;

		xrcolor2.red = 18*257;
		xrcolor2.green = 15*257;
		xrcolor2.blue = 28*257;

		xrcolor3.red = 24*257;
		xrcolor3.green = 45*257;
		xrcolor3.blue = 87*257;


		XftColor xftcolor1;
		XftColor xftcolor2;
		XftColor xftcolor3;

		XftColorAllocValue(dpy,DefaultVisual(dpy,screen),DefaultColormap(dpy,screen),&xrcolor1,&xftcolor1);
		XftColorAllocValue(dpy,DefaultVisual(dpy,screen),DefaultColormap(dpy,screen),&xrcolor2,&xftcolor2);
		XftColorAllocValue(dpy,DefaultVisual(dpy,screen),DefaultColormap(dpy,screen),&xrcolor3,&xftcolor3);

		colors->background = xftcolor2;
		colors->foreground = xftcolor1;
		colors->highlight = xftcolor3;

		//swa.background_pixel = BlackPixel(dpy, screen);
		//swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | VisibilityChangeMask;
		//swa.event_mask = ExposureMask | KeyPressMask | VisibilityChangeMask;
		swa.event_mask = ExposureMask | KeyPressMask | VisibilityChangeMask;

		win = XCreateWindow(dpy, root, winx, winy, winw, winh, 2, CopyFromParent, CopyFromParent, CopyFromParent, CWBackPixel | CWEventMask, &swa);
		XStoreName(dpy, win, "sfm");
		XClassHint xch;
		xch.res_name = "sfm";
		xch.res_class = "sfm";
		XSetClassHint(dpy, win, &xch);
		XGetWindowAttributes(dpy, win, &wa);

		XftDraw *d = XftDrawCreate(dpy, win, DefaultVisual(dpy, screen), cmap);

		//Font font = XLoadFont(dpy, "-adobe-utopia-regular-r-normal--33-240-100-100-p-180-iso8859-14");	
		
		//XGCValues sgc;
		//sgc.font = font;
		//gc = XCreateGC(dpy, win, GCFont, &sgc);
		gc = XCreateGC(dpy, win, 0, NULL);

		XMapWindow(dpy, win);

		XEvent event;

		char *dir = (char *)malloc(500*sizeof(char));
		strcpy(dir, "/home/joresg/");

		int font_asc_dsc = font->ascent + font->descent;
		int choice_dir = 0;
		int content_start = 0, num_of_displayed_dirs = wa.height/font_asc_dsc;
		XWindowAttributes rwa;
		ls_ret *subdirs = ls(dir);
		int run = 1;
		while(run && !XNextEvent(dpy, &event)){
			XGetWindowAttributes(dpy, win, &wa);
			num_of_displayed_dirs = wa.height/font_asc_dsc;
			//subdirs = ls(dir);
			//get all subdirs
			printf("is %s dir? %d\n",dir, is_dir(dir));
			//draw(dpy, screen, win, gc, wa, subdirs, choice_dir, content_start);
			switch(event.type){
				case Expose:
					printf("%s\n","EXPOSE");
					break;
				case KeyRelease: // in case autorepeat fails just ignore keyrelease events
					continue;
				case KeyPress:
					printf("%s\n","KEYPRESS");
					//printf("KeyPressDIR: %s\n", dir);
					switch(event.xkey.keycode){

						case 22:
							strcpy(dir, "/home/joresg/");
							ls_free(subdirs);
							subdirs = ls(dir);
							choice_dir = content_start = 0;
							break;
						case 24:
							//tle bo pac cleanup funkcija in cao
							run = 0;
							break;
						case 58: //make directory
							printf("mkdir\n");
							FILE *fp;
							char dir_name[1035];
							strcpy(dir_name, "");

							fp = popen("echo \"\" | dmenu -p \"dir name\" -c", "r");
							if (fp == NULL) {
							printf("Failed to run command\n" );
							exit(1);
							}

							//while (fgets(path, sizeof(path), fp) != NULL) {
							fgets(dir_name, sizeof(dir_name), fp);
							dir_name[strlen(dir_name)-1] = '\0';
							pclose(fp);
							if(strlen(dir_name) == 0){
								continue;
							}
							char *new_dir = (char *)malloc((strlen(dir)+strlen(dir_name)+1)*sizeof(char));
							strcpy(new_dir, dir);
							strcat(new_dir, dir_name);
							printf("new dir path: %s\n", new_dir);
							int status;
							status = mkdir(new_dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
							ls_free(subdirs);
							free(new_dir);
							subdirs = ls(dir);
							break;
						case 40: //remove dir or file
							printf("remove: %s%s\n", dir, subdirs->str[choice_dir]);
							char *delete_target = (char *)malloc((strlen(dir)+strlen(subdirs->str[choice_dir])+1)*sizeof(char));
							strcpy(delete_target, dir);
							strcat(delete_target, subdirs->str[choice_dir]);

							//FILE *fp;
							char conformation[6];

							char *cmd = (char *)malloc(40+strlen(subdirs->str[choice_dir])*sizeof(char));
							strcpy(cmd, "echo \"no\nyes\" | dmenu -p \"delete ");
							strcat(cmd, subdirs->str[choice_dir]);
							strcat(cmd, "?\" -c");
							printf("cmd: %s len: %ld\n", cmd, strlen(cmd));
							fp = popen(cmd, "r");
							if (fp == NULL) {
							printf("Failed to run command\n" );
							exit(1);
							}
							fgets(conformation, sizeof(conformation), fp);
							conformation[strlen(conformation)-1] = '\0';
							pclose(fp);
							printf("conformation: %s\n", conformation);
							if(!strcmp(conformation, "yes")){
								printf("deleting...");
								status = remove_file_dir(delete_target);
								printf("STATUS: %d\n", status);
							}
							free(delete_target);
							ls_free(subdirs);
							subdirs = ls(dir);
							if(choice_dir > 0 && choice_dir == subdirs->len){
								choice_dir--;
							}
							break;
						case 43: //h aka go back
							printf("%s\n","goin to parent dir");
							if(strcmp(dir,"/")){
								int substrend = find_nth_last_char(dir, '/', 2);
								if(!substrend){
									strcpy(dir, "/");
								}else{
									char *dir_substr = substr(dir, 0, substrend+1);
									strcpy(dir, dir_substr);
									printf("substrend: %d\n", substrend);
									printf("parent DIr: %s\n", dir);
									free(dir_substr);
								}
							}
							choice_dir = 0;
							content_start = 0;
							ls_free(subdirs);
							subdirs = ls(dir);
							printf("SUBDIRS LEN %d\n",subdirs->len);
							break;
						case 44: //j aka go down
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
							if(dir_empty(subdirs)){
								printf("prazen dir, skipaj...\n");
								continue;
							}
							printf("dir....%s\nsubidr.....%s\n",dir, subdirs->str[choice_dir]);
							char *dir_tmp = (char *)malloc(500*sizeof(char));
							strcpy(dir_tmp, dir);
							strcat(dir_tmp,subdirs->str[choice_dir]);
							if(is_dir(dir_tmp)){
								strcpy(dir, dir_tmp);
								strcat(dir,"/");
								choice_dir = 0;
								content_start = 0;
								ls_free(subdirs);
								subdirs = ls(dir);
							}
							else{
								//check if file needs to be opened in a terminal
								//if so spawn a shell and execute program in it
								int status;
								//first get the corresponding .desktop file
								//escape potential special characters
								dir_tmp = handle_special_chars(dir_tmp);
							printf("popravljeno: %s\n", dir_tmp);
							char *query_filetype_cmd = (char *)malloc((25+strlen(dir_tmp))*sizeof(char));
							strcpy(query_filetype_cmd, "xdg-mime query filetype ");
							strcat(query_filetype_cmd, dir_tmp);
							printf("query filetype cmd: %s\n", query_filetype_cmd);

							FILE *fp;
							char filetype[1035];

							fp = popen(query_filetype_cmd, "r");
							if (fp == NULL) {
							printf("Failed to run command\n" );
							exit(1);
							}

							//while (fgets(path, sizeof(path), fp) != NULL) {
							fgets(filetype, sizeof(filetype), fp);
							filetype[strlen(filetype)-1] = '\0';
							pclose(fp);
							printf("filetype: %s\n", filetype);

							char *desktop_file_cmd = (char *)malloc((24+strlen(filetype))*sizeof(char));
							strcpy(desktop_file_cmd, "xdg-mime query default ");
							strcat(desktop_file_cmd, filetype);

							char df[1035];
							fp = popen(desktop_file_cmd, "r");
							if(fp == NULL){
								printf("Failed to run command\n");
								exit(1);
							}
							fgets(df, sizeof(df), fp);
							df[strlen(df)-1] = '\0';
							pclose(fp);

							//now check if it needs to be opened in the terminal
							char *is_term_cmd = (char *)malloc((30+strlen(df)+33)*sizeof(char));
							strcpy(is_term_cmd, "cat /usr/share/applications/");
							strcat(is_term_cmd, df);
							strcat(is_term_cmd, " | grep Terminal | cut -d'=' -f2");

							char is_term[7];
							fp = popen(is_term_cmd, "r");
							if(fp == NULL){
								printf("Failed to run cmd\n");
								exit(1);
							}
							fgets(is_term, sizeof(is_term), fp);
							is_term[strlen(is_term)-1] = '\0';
							printf("is_term: %s\n", is_term);
							if(!strcmp(is_term, "true")){
								char *cmd = (char *)malloc((17+strlen(dir_tmp))*sizeof(char));
								strcpy(cmd, "st -e xdg-open ");
								strcat(cmd, dir_tmp);
								strcat(cmd, " &");
								status = system(cmd);
								printf("status: %d\n", status);
								free(cmd);


							}else{
								char *cmd = (char *)malloc((11+strlen(dir_tmp))*sizeof(char));
								strcpy(cmd, "xdg-open ");
								strcat(cmd, dir_tmp);
								strcat(cmd, " &");
								printf("CMD: %s\n", cmd);
								status = system(cmd);
								printf("status: %d\n", status);
								free(cmd);

							}
							free(query_filetype_cmd);
							free(desktop_file_cmd);
							free(is_term_cmd);
							free(dir_tmp);
						}
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
				printf("event type: %d\n", event.type);
				continue;
				break;
		}
		draw(dpy, screen, win, gc, d, font, colors, wa, subdirs, choice_dir, content_start, dir);
	}
	XftColorFree(dpy,DefaultVisual(dpy,0),DefaultColormap(dpy,0),&xftcolor1);
	XftColorFree(dpy,DefaultVisual(dpy,0),DefaultColormap(dpy,0),&xftcolor2);
	XftColorFree(dpy,DefaultVisual(dpy,0),DefaultColormap(dpy,0),&xftcolor3);
	free(subdirs);
	return(0);
}
