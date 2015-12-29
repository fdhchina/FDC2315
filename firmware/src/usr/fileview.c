#include "hal.h"

#include <string.h>

#include "ff.h"
#include "lcd19264.h"
#include "fileview.h"
#include "player.h"
#include "task.h"
#include "menu.h"
#include "usrkey.h"
#include "ch375.h"


void strfmt(char * str,char * fmt,...);



void GetRealName(char *pEntryName, const char *pOrgName) 
{
  int i;

	for(i=0;i<strlen(pOrgName);i++)
		if(pOrgName[i]!=' ')
			*pEntryName++= pOrgName[i];
	*pEntryName= '\0';
}

// ****************************************************************************
//		返回文件名filename的路径
// *****************************************************************************
void ExtractFilePath(const char * filename,char *path)
{	
	char *pos; 
	int len;
	
	*path='\0';
	pos=strrchr(filename,'\\');
	if(pos==NULL) return ;
	if(strlen(filename)<5) return ;		//文件名长度小于5,返回FALSE 
	len= pos-filename;
	strncpy(path,filename,len);
	path[len]='\0';
}



// ****************************************************************************
//		返回文件filename的扩展名
// *****************************************************************************
char * ExtractFileExt(const char * filename)
{	
	if(strlen(filename)<5) return(FALSE);		//文件名长度小于5,返回FALSE 
	return(strrchr(filename,'.'));
}

// ****************************************************************************
//		返回文件pathname的主文件名
// *****************************************************************************
void ExtractFileName(u8 includeext, const char *path_name,char *name)
{
	char *len,*len1;
	
	if(includeext)
		len= (char *)path_name+strlen(path_name);
	else
		len= strrchr(path_name,'.');
	len1= strrchr(path_name,'\\');
	if(len1==NULL)	len1=(char *)path_name;
			else	len1++;
	if(len==NULL)	len=(char *)path_name+strlen(path_name);
			else	len--;
	strncpy(name,len1,(len-len1+1));
	name[len-len1+1]='\0';
}

// *****************************************************************************
//          ******************取乐曲文件长度************************
// *****************************************************************************
#include "mpegaudio.h"
#include "wavfile.h"
#include "wmafile.h"

int GetAudioLength(char *filename)
{
	char *ext;
	TMPEGaudio mp3;
	TWAVfile	wav;
	TWMAfile	wma;
	
	ext= ExtractFileExt(filename);
	if(memcmp(ext,".MP3",4)==0)
	{
		mp3_ReadFromFile(&mp3,filename);
		return mp3.FDuration;
	}
	if(memcmp(ext,".WAV",4)==0)
	{
		wav_ReadFromFile(&wav,filename);
		return wav.FDuration;
	}
	if(memcmp(ext,".WMA",4)==0)
	{
		wma_ReadFromFile(&wma,filename);
		return wma.FDuration;
	}
	return 0;
}

// *****************************************************************************



#include "44blib.h"

// ****************************************************************************
//		取得短文件名shortname对应的长文件名,返回 在longname中
// *****************************************************************************
int GetLongFileName(char *shortname,char *longname,u8 getlen,unsigned long dirpos) 
{
	FS_DIR *dirp;
  	struct FS_DIRENT *direntp;
	char pathname[50], filename[25], *pBuf,i=0;
	int length=0;
	
	*longname='\0';
	if (shortname==NULL) return 0;
	if(	getlen	)
		length=GetAudioLength(shortname);
	ExtractFilePath(shortname,pathname);		
	ExtractFileName(1,shortname,filename);

 	dirp = FS_OpenDir(pathname);
	dirp->dirpos= dirpos;
  	if (dirp) {
    	do {
      		direntp = FS_ReadDir(dirp);
      		if (direntp) {
	    		if(direntp->FAT_DirAttr==0x10) continue;			//是目录则继续
      			pBuf=filename;
      			for(i=0;i<11;i++){
					if(direntp->short_name[i]!=*pBuf)break;
					else pBuf++;
				}
      			if(i==11)
      			{
	      			strcpy(longname,direntp->long_name);
      				break;						//找到匹配的短文件名
      			}
      		}
    	} while (direntp);
    	FS_CloseDir(dirp);
  	}
  	longname[strlen(longname)]='\0';
  	//if(i==0) strcpy(longname,shortname);
	//Uart0_Printf("%s Length:  %d Seconds\n",longname,length);
	return length;
}


DirName_Buf	udisk_audiolist;

void ClearDirNameBuf(void)
{
	memset(&udisk_audiolist,0,sizeof(udisk_audiolist));
}

U16  GetUdiskRootFileList(void)
{
	char path[30],i;
	U16 count;
	for(i=0;i<ch375_max_lun+1;i++)
	{
		strfmt(path,"ud:%d:\\", ch375_max_lun-i);
		udisk_audiolist.count=0;	
		count= GetFileList(path,&udisk_audiolist,0,0);
		if(count) return count;
	}
	return 0;
}


U8 GetFileList(char *path,PDirName_Buf list,U8 pathindex, u8 addpath)
{
	FS_DIR *dirp;
  	struct FS_DIRENT *direntp;
	S16	i;
	U8	pathlen;
	U16 count;
	char *ext;
	FS_FILE	*file;
	char		filename[255];

	count=list->count;				//总数
	pathlen = strlen(path); // 计算文件名长度，指向结束符 
	strcpy(list->path[pathindex],path);
  	dirp = FS_OpenDir(path);
  	if (dirp) {
    	do {
      		direntp = FS_ReadDir(dirp);
      		if (direntp) {
	    		if(direntp->FAT_DirAttr==0x10) continue;
	
				ext=ExtractFileExt((char *)direntp->short_name);
				if ( 	(((*(ext+1)) =='M') && ((*(ext+2)) =='P')&&((*(ext+3)) =='3')) 
					 || (((*(ext+1)) =='W') && ((*(ext+2)) =='M')&&((*(ext+3)) =='A')) 
					 || (((*(ext+1)) =='W') && ((*(ext+2)) =='A')&&((*(ext+3)) =='V')) 
				   )
				{
					strcpy(filename,path);
					strcat(filename, (char *)direntp->short_name);
					
					file= FS_FOpen(filename, "r");
					if( (file) && (file->size>0x1000))
					{
						if(addpath)
						{
							strcpy(list->filelist[count],path);
							strcat(list->filelist[count], (char *)direntp->short_name);
						}else
							strcpy(list->filelist[count], (char *)direntp->short_name);
					
						if(list==&fix_audiolist.AudioList)
						{
							i= GetSongIndex(list->filelist[count]);
							if(i==-1) goto find_continue;
							SetFixSongIndex(i,count);
						}else
							if(list==&rec_audiolist.AudioList)
							{
								i= GetRecFileIndex(list->filelist[count]);
								if(i!=-1)
									SetRecFileIndex(i,count);
							} else
							if(list==&ext_audiolist.AudioList)
							{
								i= GetSongIndex(list->filelist[count]);
								if(i!=-1) goto find_continue;
							}
						list->path_index[count]= pathindex;
						list->dirpos[count]= dirp->dirpos-32;
						count++;
						if (list==&fix_audiolist.AudioList) 
						{
							if(count==FIX_SONG_MAX)	break;
						}else
							if(count==EXT_SONG_MAX) break;
					}
find_continue:		if(file)
						FS_FClose(file);
				}
      		}
    	} while (direntp);
    	FS_CloseDir(dirp);
	}
	list->count=count;
	return(count);
}

void GetPathToFileList(char *path, PDirName_Buf fixlist, PDirName_Buf extlist, U8 pathindex, u8 addpath)
{
	FS_DIR *dirp;
  	struct FS_DIRENT *direntp;
	S16	i;
	U8	pathlen;
	U16 fixcount, extcount;
	char *ext;
	FS_FILE	*file;
	char		filename[255];

	fixcount=fixlist->count;				//总数
	extcount= extlist->count;
	pathlen = strlen(path); // 计算文件名长度，指向结束符 
	strcpy(fixlist->path[pathindex],path);
	strcpy(extlist->path[pathindex],path);
  	dirp = FS_OpenDir(path);
  	if (dirp) {
    	do {
      		direntp = FS_ReadDir(dirp);
      		if (direntp) {
	    		if(direntp->FAT_DirAttr==0x10) continue;
	
				ext=ExtractFileExt((char *)direntp->short_name);
				if ( 	(((*(ext+1)) =='M') && ((*(ext+2)) =='P')&&((*(ext+3)) =='3')) 
					 || (((*(ext+1)) =='W') && ((*(ext+2)) =='M')&&((*(ext+3)) =='A')) 
					 || (((*(ext+1)) =='W') && ((*(ext+2)) =='A')&&((*(ext+3)) =='V')) 
				   )
				{
					strcpy(filename,path);
					strcat(filename, (char *)direntp->short_name);
					
					file= FS_FOpen(filename, "r");
					if( (file) && (file->size>0x1000))
					{
						i= GetSongIndex(filename);
						if( (i==-1)||(i>247) )		// 扩序曲目
						{
							if(addpath)
							{
								strcpy(extlist->filelist[extcount],path);
								strcat(extlist->filelist[extcount], (char *)direntp->short_name);
							}else
								strcpy(extlist->filelist[extcount], (char *)direntp->short_name);

							extlist->path_index[extcount]= pathindex;
							extlist->dirpos[extcount]= dirp->dirpos-32;
							extcount++;
						}else
						{
							if(addpath)
							{
								strcpy(fixlist->filelist[fixcount],path);
								strcat(fixlist->filelist[fixcount], (char *)direntp->short_name);
							}else
								strcpy(fixlist->filelist[fixcount], (char *)direntp->short_name);

							SetFixSongIndex(i,fixcount);
							fixlist->path_index[fixcount]= pathindex;
							fixlist->dirpos[fixcount]= dirp->dirpos-32;
							fixcount++;
						}
					}
					if(file)
						FS_FClose(file);
				}
      		}
    	} while (direntp);
    	FS_CloseDir(dirp);
	}
	fixlist->count=fixcount;
	extlist->count= extcount;
}


//	****************************************************
//	目录文件显示
//	****************************************************
#define VIEWNAME_CHARNUMS				26
#define VIEWNAME_SHOWROWS				3
typedef struct _DIRVIEW_BUF
{
	U16 index;
	char buf[255];
	rollstr_t		rollstr;
}DirView_Buf;

static struct
{
	S8 	show_name_start;		//开始显示项编号
	S16	file_index;				//当前文件编号
	DirView_Buf name_buf[VIEWNAME_SHOWROWS];		//显示名称数据
	queue_p 		queue;
}dirname_viewbuf;

DirName_Buf	view_audiolist;

void Show_NullFileName(void)
{
	Putstr(57,32,"没有曲目文件!",13,0);
}

/*
void Show_FileName(void)
{
	U8 i;
	
	for(i=0;i<3;i++)
		Putstr(3,(i<<4)+16,dirname_viewbuf.name_buf[i].buf,31,dirname_viewbuf.name_buf[i].index==dirname_viewbuf.file_index);
		
}

void Show_Directory(char *path, S8 inc) 
{
	U8 len,i,j,k,m=0,n,c=0;
	char shortname[255],longname[50],filename[13];

	//if(strcmp(udisk_audiolist.path,path)!=0)
	if(inc==0)
	{
		memset(&view_audiolist,0,sizeof(view_audiolist));
		strcpy(view_audiolist.path,path);
		GetFileList(path,&view_audiolist,0);
		memset(&dirname_viewbuf,0,sizeof(dirname_viewbuf));
	}
	if(view_audiolist.count==0)
	{
		Show_NullFileName();
		return;
	}
	dirname_viewbuf.file_index+=inc;
	if(dirname_viewbuf.file_index>=view_audiolist.count) dirname_viewbuf.file_index= view_audiolist.count-1;
	if(dirname_viewbuf.file_index<0) dirname_viewbuf.file_index= 0;

	if(inc==0)	
	{
		 c=3;k=0;m=0
	}else 
		if((dirname_viewbuf.file_index>dirname_viewbuf.name_buf[2].index)||(dirname_viewbuf.file_index<dirname_viewbuf.name_buf[0].index))
		{
			k=dirname_viewbuf.file_index;
			c=k+1;
			if(inc==1)
			{
				memcpy(&dirname_viewbuf.name_buf[0],&dirname_viewbuf.name_buf[1],sizeof(DirView_Buf));
				memcpy(&dirname_viewbuf.name_buf[1],&dirname_viewbuf.name_buf[2],sizeof(DirView_Buf));
				m=3;
			}else
			{
				memcpy(&dirname_viewbuf.name_buf[2],&dirname_viewbuf.name_buf[1],sizeof(DirView_Buf));
				memcpy(&dirname_viewbuf.name_buf[1],&dirname_viewbuf.name_buf[0],sizeof(DirView_Buf));
				m=1;
			}
		}
	for(j=k;j<c;j++)
	{
		n=(m==0)?(j-k):(m-1);
		memset(&dirname_viewbuf.name_buf[n],0,sizeof(DirView_Buf));		
		if(j>=view_audiolist.count) break;
		strcpy(shortname,view_audiolist.path);
		strcat(shortname, view_audiolist.filelist[j]);
		GetLongFileName(shortname,longname);	
		
	  	if(strlen(longname)==0)
	  	{
	  		memset(filename,0,13);
  			ExtractFileName(1,shortname,filename);
	  		GetRealName(longname, filename);
	  	}
		
		len= strlen(longname);
		len= (len>VIEWNAME_CHARNUMS-5)?(VIEWNAME_CHARNUMS-5):len;
		strfmt(dirname_viewbuf.name_buf[n].buf,"[%03d]",j+1);
		strncat(dirname_viewbuf.name_buf[n].buf,longname,len);
		for(i=len+5;i<VIEWNAME_CHARNUMS;i++)
			dirname_viewbuf.name_buf[n].buf[i]=' ';
		/ *if(file_attr==0x10)
		{
			dirname_viewbuf[j-k].buf[VIEWNAME_CHARNUMS-3]= '<';
			dirname_viewbuf[j-k].buf[VIEWNAME_CHARNUMS-2]= 'D';
			dirname_viewbuf[j-k].buf[VIEWNAME_CHARNUMS-1]= '>';
		}* /
		dirname_viewbuf.name_buf[n].index= j;
	}
	Show_FileName();
}

*/
// 显示在某一行的表
static U8 showrow_tab[3][3]={{0,1,2},{2,0,1},{1,2,0}};

void Show_FileName(void)
{
	U8 i,j,row,len;
	char name_id[255];
	
	for(i=0;i<VIEWNAME_SHOWROWS;i++)
	{
		if(dirname_viewbuf.name_buf[i].index==0xFFFF) continue;
		len= strlen(dirname_viewbuf.name_buf[i].buf);
		row=(showrow_tab[dirname_viewbuf.show_name_start][i]<<4)+16;
		strfmt(name_id,"[%03d]",dirname_viewbuf.name_buf[i].index+1);
		Putstr(3,row,name_id,5,0);
		if((len<=VIEWNAME_CHARNUMS)||(dirname_viewbuf.name_buf[i].rollstr.delay==ROLLNAME_DELAY))
		{
			dirname_viewbuf.name_buf[i].rollstr.delay--;
			strcpy(name_id,dirname_viewbuf.name_buf[i].buf);
			for(j=len;j<VIEWNAME_CHARNUMS;j++)
				name_id[j]=' ';
			Putstr(33,row,name_id,VIEWNAME_CHARNUMS,  dirname_viewbuf.name_buf[i].index== dirname_viewbuf.file_index);
		}	
		else
		{
			dirname_viewbuf.name_buf[i].rollstr.instead= dirname_viewbuf.name_buf[i].index== dirname_viewbuf.file_index;
			dirname_viewbuf.name_buf[i].rollstr.row= row;
			RollShowStr(&dirname_viewbuf.name_buf[i].rollstr);
		}
	}
/*
	U8 len=dirname_viewbuf.show_name_start,i;
	
	for(i=0;i<VIEWNAME_SHOWROWS;i++)
	{
		Putstr(3,(i<<4)+16,dirname_viewbuf.name_buf[len].buf,VIEWNAME_CHARNUMS,dirname_viewbuf.name_buf[len].index==dirname_viewbuf.file_index);
		len++;
		if(len>=VIEWNAME_SHOWROWS) len=0;
	}		
	*/
}

u8  fileindexisbuf()
{
	U8  i;

	for (i=0;i<VIEWNAME_SHOWROWS;i++)
		if( dirname_viewbuf.name_buf[i].index== dirname_viewbuf.file_index) break;
	return i<VIEWNAME_SHOWROWS;
}

// *********************************************************************************
// 循环方式 2008-08-22
// *********************************************************************************
void Show_AudioList(DirName_Buf* list,S8 inc)
{
	S16 c=0,len,i,j,k;
	U8 m=0,n;
	char shortname[255],longname[255],filename[13];

	if(inc==0)
	{
		memset(&dirname_viewbuf,0,sizeof(dirname_viewbuf));
	}
	if(list->count==0)
	{
		Show_NullFileName();
		return;
	}
	dirname_viewbuf.file_index+=inc;
	if(dirname_viewbuf.file_index>=list->count) dirname_viewbuf.file_index= 0;	// list->count-1;
	if(dirname_viewbuf.file_index<0) dirname_viewbuf.file_index= list->count-1;	//	0;

	if(inc==0)	
	{
		 c=VIEWNAME_SHOWROWS;k=0;m=0;
	}else 
	{
		if(inc==1)
		{
			if(	(list->count>= VIEWNAME_SHOWROWS)&&(!fileindexisbuf())	)
			{
				m=0x80 |dirname_viewbuf.show_name_start;
				k=dirname_viewbuf.file_index;
				c=k+1;
				dirname_viewbuf.show_name_start ++;
				if(dirname_viewbuf.show_name_start == VIEWNAME_SHOWROWS) dirname_viewbuf.show_name_start=0;
			}
		}

		if(inc==-1)
		{

			if(	(list->count>= VIEWNAME_SHOWROWS)&&(!fileindexisbuf())	)
			{
				k=dirname_viewbuf.file_index;
				c=k+1;
				dirname_viewbuf.show_name_start--;
				if(dirname_viewbuf.show_name_start < 0) dirname_viewbuf.show_name_start=VIEWNAME_SHOWROWS-1;
				m=0x80 | dirname_viewbuf.show_name_start;
			}
		}
	}
	for(j=k;j<c;j++)
	{
		n=(m&0x80)?(m&0x7F):(j-k);
		memset(&dirname_viewbuf.name_buf[n],0,sizeof(DirView_Buf));	
		dirname_viewbuf.name_buf[n].index=0xFFFF;
		if(j>=list->count) continue;
		strcpy(shortname,list->path[list->path_index[j]]);
		strcat(shortname, list->filelist[j]);
		GetLongFileName(shortname,longname,FALSE,list->dirpos[j]);	
		
	  	if(strlen(longname)==0)
	  	{
	  		memset(filename,0,13);
  			ExtractFileName(1,shortname,filename);
	  		GetRealName(longname, filename);
	  	}
		//strfmt(dirname_viewbuf.name_buf[n].buf,"[%03d]",j+1);
		strcpy(dirname_viewbuf.name_buf[n].buf,longname);
		dirname_viewbuf.name_buf[n].index= j;

		strfmt(shortname,"[%03d]",j+1);
		Putstr(3,(showrow_tab[dirname_viewbuf.show_name_start][n]<<4)+16,shortname,5,0);

		len= strlen(dirname_viewbuf.name_buf[n].buf);
		len= (len>VIEWNAME_CHARNUMS)?(VIEWNAME_CHARNUMS):len;
		strncpy(shortname,dirname_viewbuf.name_buf[n].buf,len);
		for(i=len;i<VIEWNAME_CHARNUMS;i++)
			shortname[i]=' ';
		shortname[VIEWNAME_CHARNUMS]=0;

		Putstr(33,(showrow_tab[dirname_viewbuf.show_name_start][n]<<4)+16,shortname,VIEWNAME_CHARNUMS,j==dirname_viewbuf.file_index);
		remove_queue(Timer_queue,dirname_viewbuf.queue);
		dirname_viewbuf.queue= add_queue(Timer_queue);
		if(dirname_viewbuf.queue)
		{
			dirname_viewbuf.name_buf[n].rollstr.col= 33;
//			dirname_viewbuf.name_buf[n].rollstr.row= (showrow_tab[dirname_viewbuf.show_name_start][n]<<4)+16;
//			dirname_viewbuf.name_buf[n].rollstr.instead= j==dirname_viewbuf.file_index;
			dirname_viewbuf.name_buf[n].rollstr.pos=0;
			dirname_viewbuf.name_buf[n].rollstr.delay=ROLLNAME_DELAY;
			dirname_viewbuf.name_buf[n].rollstr.showlen= VIEWNAME_CHARNUMS;
			dirname_viewbuf.name_buf[n].rollstr.str= dirname_viewbuf.name_buf[n].buf;
			dirname_viewbuf.queue->func= Show_FileName;
		}
	}
	Show_FileName();
}

void Show_Directory(char *path, S8 inc) 
{
	S16 c=0,len,i,j,k;
	U8 m=0,n;
	char shortname[255],longname[255],filename[13];

	//if(strcmp(view_audiolist.path,path)!=0)
	if(inc==0)
	{
		memset(&view_audiolist,0,sizeof(view_audiolist));
		strcpy(view_audiolist.path[0],path);
		GetFileList(path,&view_audiolist,0,0);
		memset(&dirname_viewbuf,0,sizeof(dirname_viewbuf));
	}
	if(view_audiolist.count==0)
	{
		Show_NullFileName();
		return;
	}
	dirname_viewbuf.file_index+=inc;
	if(dirname_viewbuf.file_index>=view_audiolist.count) dirname_viewbuf.file_index= 0;	// view_audiolist.count-1;
	if(dirname_viewbuf.file_index<0) dirname_viewbuf.file_index= view_audiolist.count-1;	//	0;

	if(inc==0)	
	{
		 c=VIEWNAME_SHOWROWS;k=0;m=0;
	}else 
	{
		if(inc==1)
		{
			if(	(view_audiolist.count>= VIEWNAME_SHOWROWS)&&(!fileindexisbuf())	)
			{
				m=0x80 |dirname_viewbuf.show_name_start;
				k=dirname_viewbuf.file_index;
				c=k+1;
				dirname_viewbuf.show_name_start ++;
				if(dirname_viewbuf.show_name_start == VIEWNAME_SHOWROWS) dirname_viewbuf.show_name_start=0;
			}
		}

		if(inc==-1)
		{

			if(	(view_audiolist.count>= VIEWNAME_SHOWROWS)&&(!fileindexisbuf())	)
			{
				k=dirname_viewbuf.file_index;
				c=k+1;
				dirname_viewbuf.show_name_start--;
				if(dirname_viewbuf.show_name_start < 0) dirname_viewbuf.show_name_start=VIEWNAME_SHOWROWS-1;
				m=0x80 | dirname_viewbuf.show_name_start;
			}
		}
	}
	for(j=k;j<c;j++)
	{
		n=(m&0x80)?(m&0x7F):(j-k);
		memset(&dirname_viewbuf.name_buf[n],0,sizeof(DirView_Buf));		
		if(j>=view_audiolist.count) break;
		strcpy(shortname,view_audiolist.path[0]);
		strcat(shortname, view_audiolist.filelist[j]);
		GetLongFileName(shortname,longname,FALSE,view_audiolist.dirpos[j]);	
		
	  	if(strlen(longname)==0)
	  	{
	  		memset(filename,0,13);
  			ExtractFileName(1,shortname,filename);
	  		GetRealName(longname, filename);
	  	}
		len= strlen(longname);
		len= (len>VIEWNAME_CHARNUMS-5)?(VIEWNAME_CHARNUMS-5):len;
		strfmt(dirname_viewbuf.name_buf[n].buf,"[%03d]",j+1);
		strncat(dirname_viewbuf.name_buf[n].buf,longname,len);
		for(i=len+5;i<VIEWNAME_CHARNUMS;i++)
			dirname_viewbuf.name_buf[n].buf[i]=' ';
		dirname_viewbuf.name_buf[n].buf[VIEWNAME_CHARNUMS]=0;
		dirname_viewbuf.name_buf[n].index= j;
	}
	Show_FileName();
}


/*
// *********************************************************************************
// 非循环方式
// *********************************************************************************
void Show_AudioList(DirName_Buf* list,S8 inc)
{
	S16 len,i,j,k;
	U8 m=0,c=0,n;
	char shortname[255],longname[255],filename[13];

	if(inc==0)
	{
		memset(&dirname_viewbuf,0,sizeof(dirname_viewbuf));
	}
	if(list->count==0)
	{
		Show_NullFileName();
		return;
	}
	dirname_viewbuf.file_index+=inc;
	if(dirname_viewbuf.file_index>=list->count) dirname_viewbuf.file_index= list->count-1;
	if(dirname_viewbuf.file_index<0) dirname_viewbuf.file_index= 0;

	if(inc==0)	
	{
		 c=VIEWNAME_SHOWROWS;k=0;m=0;
	}else 
	{
		if(inc==1)
		{
			k= (dirname_viewbuf.show_name_start+VIEWNAME_SHOWROWS-1) % VIEWNAME_SHOWROWS;
			if((list->count>= VIEWNAME_SHOWROWS)
				&& (dirname_viewbuf.file_index>dirname_viewbuf.name_buf[k].index))
			{
				k=dirname_viewbuf.file_index;
				c=k+1;
				m=0x80 | dirname_viewbuf.show_name_start;
				dirname_viewbuf.show_name_start ++;
				if(dirname_viewbuf.show_name_start == VIEWNAME_SHOWROWS) dirname_viewbuf.show_name_start=0;
			}
		}
		if(inc==-1)
		{
			if(dirname_viewbuf.file_index<dirname_viewbuf.name_buf[dirname_viewbuf.show_name_start].index)
			{
				k=dirname_viewbuf.file_index;
				c=k+1;
				dirname_viewbuf.show_name_start--;
				if(dirname_viewbuf.show_name_start < 0) dirname_viewbuf.show_name_start=VIEWNAME_SHOWROWS-1;
				m=0x80 | dirname_viewbuf.show_name_start;
			}
		}
	}
	for(j=k;j<c;j++)
	{
		n=(m&0x80)?(m&0x7F):(j-k);
		memset(&dirname_viewbuf.name_buf[n],0,sizeof(DirView_Buf));		
		if(j>=list->count) break;
		strcpy(shortname,list->path[list->path_index[j]]);
		strcat(shortname, list->filelist[j]);
		GetLongFileName(shortname,longname,FALSE,list->dirpos[j]);	
		
	  	if(strlen(longname)==0)
	  	{
	  		memset(filename,0,13);
  			ExtractFileName(1,shortname,filename);
	  		GetRealName(longname, filename);
	  	}
		len= strlen(longname);
		len= (len>VIEWNAME_CHARNUMS-5)?(VIEWNAME_CHARNUMS-5):len;
		strfmt(dirname_viewbuf.name_buf[n].buf,"[%03d]",j+1);
		strncat(dirname_viewbuf.name_buf[n].buf,longname,len);
		for(i=len+5;i<VIEWNAME_CHARNUMS;i++)
			dirname_viewbuf.name_buf[n].buf[i]=' ';
		dirname_viewbuf.name_buf[n].buf[VIEWNAME_CHARNUMS]=0;
		dirname_viewbuf.name_buf[n].index= j;
	}
	Show_FileName();
}


void Show_Directory(char *path, S8 inc) 
{
	S16 len,i,j,k;
	U8 m=0,c=0,n;
	char shortname[255],longname[255],filename[13];

	//if(strcmp(view_audiolist.path,path)!=0)
	if(inc==0)
	{
		memset(&view_audiolist,0,sizeof(view_audiolist));
		strcpy(view_audiolist.path[0],path);
		GetFileList(path,&view_audiolist,0,0);
		memset(&dirname_viewbuf,0,sizeof(dirname_viewbuf));
	}
	if(view_audiolist.count==0)
	{
		Show_NullFileName();
		return;
	}
	dirname_viewbuf.file_index+=inc;
	if(dirname_viewbuf.file_index>=view_audiolist.count) dirname_viewbuf.file_index= view_audiolist.count-1;
	if(dirname_viewbuf.file_index<0) dirname_viewbuf.file_index= 0;

	if(inc==0)	
	{
		 c=VIEWNAME_SHOWROWS;k=0;m=0;
	}else 
	{
		if(inc==1)
		{
			k= (dirname_viewbuf.show_name_start+VIEWNAME_SHOWROWS-1) % VIEWNAME_SHOWROWS;
			if((view_audiolist.count>= VIEWNAME_SHOWROWS)
				&& (dirname_viewbuf.file_index>dirname_viewbuf.name_buf[k].index))
			{
				k=dirname_viewbuf.file_index;
				c=k+1;
				//memcpy(&dirname_viewbuf[0],&dirname_viewbuf[1],sizeof(DirView_Buf));
				//memcpy(&dirname_viewbuf[1],&dirname_viewbuf[2],sizeof(DirView_Buf));
				m=0x80 | dirname_viewbuf.show_name_start;
				dirname_viewbuf.show_name_start ++;
				if(dirname_viewbuf.show_name_start == VIEWNAME_SHOWROWS) dirname_viewbuf.show_name_start=0;
			}
		}
		if(inc==-1)
		{
			if(dirname_viewbuf.file_index<dirname_viewbuf.name_buf[dirname_viewbuf.show_name_start].index)
			{
				//memcpy(&dirname_viewbuf[2],&dirname_viewbuf[1],sizeof(DirView_Buf));
				//memcpy(&dirname_viewbuf[1],&dirname_viewbuf[0],sizeof(DirView_Buf));
				k=dirname_viewbuf.file_index;
				c=k+1;
				dirname_viewbuf.show_name_start--;
				if(dirname_viewbuf.show_name_start < 0) dirname_viewbuf.show_name_start=VIEWNAME_SHOWROWS-1;
				m=0x80 | dirname_viewbuf.show_name_start;
			}
		}
	}
	for(j=k;j<c;j++)
	{
		n=(m&0x80)?(m&0x7F):(j-k);
		memset(&dirname_viewbuf.name_buf[n],0,sizeof(DirView_Buf));		
		if(j>=view_audiolist.count) break;
		strcpy(shortname,view_audiolist.path[0]);
		strcat(shortname, view_audiolist.filelist[j]);
		GetLongFileName(shortname,longname,FALSE,view_audiolist.dirpos[j]);	
		
	  	if(strlen(longname)==0)
	  	{
	  		memset(filename,0,13);
  			ExtractFileName(1,shortname,filename);
	  		GetRealName(longname, filename);
	  	}
		len= strlen(longname);
		len= (len>VIEWNAME_CHARNUMS-5)?(VIEWNAME_CHARNUMS-5):len;
		strfmt(dirname_viewbuf.name_buf[n].buf,"[%03d]",j+1);
		strncat(dirname_viewbuf.name_buf[n].buf,longname,len);
		for(i=len+5;i<VIEWNAME_CHARNUMS;i++)
			dirname_viewbuf.name_buf[n].buf[i]=' ';
		dirname_viewbuf.name_buf[n].buf[VIEWNAME_CHARNUMS]=0;
		dirname_viewbuf.name_buf[n].index= j;
	}
	Show_FileName();
}
*/


//  ********************************************************************************
// 任务备份及恢复
//  ********************************************************************************
// *************************************************************
//任务文件管理
// *************************************************************
//查找 任务文件是否存在,找到则打开
u8 ReadTaskFile(void)
{
	FS_FILE * file;
	int x;
	u8 b;
#ifdef _NEW_TASK_
	ResetTaskData();
#else
	ResetTaskList();
#endif

	file= FS_FOpen(TASKFILENAME,"rb");

	if(file)
	{
#ifdef _NEW_TASK_
		x=FS_FRead(&tdTask,1,TASK_DATA_SIZE,file);
		b= (x==TASK_DATA_SIZE);
		if(b)
			CheckTaskData();
#else
		x=FS_FRead(TaskGroupList,1,TASKGROUPLIST_SIZE,file);
		b= (x==TASKGROUPLIST_SIZE);
		if(b)
			CheckTaskData();
#endif
		FS_FClose(file);
		CreateTGIndex();		//建立任务组顺序索引
	}
	return b;
}

// *************************************************************
//保存任务到文件
u8 SaveTaskFile(void)
{
	FS_FILE * file;
	int x=-1;

#ifdef _NEW_TASK_
	tdTask.Head= TASK_HEAD_STR;
#endif	
	CreateTGIndex();		//建立任务组顺序索引
	file= FS_FOpen(TASKFILENAME,"wb");

	if(file)
	{
#ifdef _NEW_TASK_
		x=FS_FWrite(&tdTask,1,TASK_DATA_SIZE,file);
#else
		x=FS_FWrite(TaskGroupList,1,TASKGROUPLIST_SIZE,file);
#endif
		FS_FClose(file);
	}

#ifdef _GMT2000_
#if	(NANDFLASH_PARTITION_MAXUNIT==1)
 	flash_FlushCache(1,0xFF);
#else
 	flash_FlushCache(0,0xFF);
#endif
#endif
	
#ifdef _NEW_TASK_
	return (x==TASK_DATA_SIZE);
#else
	return x==TASKGROUPLIST_SIZE;
#endif
}

#ifdef _NEW_TASK_
#define TASKLIST_BEGINSECTOR	0x100000		//任务数据备份在Nor_Flash的0x1E0000开始的128K中
#else
#define TASKLIST_BEGINSECTOR	0x100000		//任务数据备份在Nor_Flash的0x1F0000开始的8K中
#endif

void RestoreTaskList(void)
{
#ifdef _NEW_TASK_
	if(*(uint32 *)TASKLIST_BEGINSECTOR!= TASK_HEAD_STR) return;
	FlashRead(TASKLIST_BEGINSECTOR,(unsigned short *)&tdTask,TASK_DATA_SIZE);
#else
	FlashRead(TASKLIST_BEGINSECTOR,(unsigned short *)TaskGroupList,TASKGROUPLIST_SIZE);
#endif
	CheckTaskData();
}

void BackupTaskList(void)
{
	int i=3;
#ifdef _NEW_TASK_
	while(SectorProg(TASKLIST_BEGINSECTOR,(uint16 *)&tdTask,TASK_DATA_SIZE)&&--i);
#else
	while(SectorProg(TASKLIST_BEGINSECTOR,(uint16 *)TaskGroupList,TASKGROUPLIST_SIZE)&&--i);
#endif
	
}

//  ********************************************************************************
// U盘文件复制
//  ********************************************************************************

u8 CopyFile(char *source,char *des)
{
	FS_FILE *fs,*fd;
	int x=-1,fs_size=1,cpy_size=0, read_size;
	char buf[0x10000];
	
	fs= FS_FOpen(source,"rb");
	if(fs)
	{
		fd= FS_FOpen(des,"wb");
		if(fd)
		{
			fs_size= fs->size;
			while(cpy_size<fs_size)
			{
				read_size= (fs_size-cpy_size>0x10000)?0x10000:(fs_size-cpy_size);
				x= FS_FRead(buf,1,read_size,fs);
				if(x<=0) break;
				x= FS_FWrite(buf,1,x,fd);
				if(x<=0) break;
				cpy_size+= x;
			}
			FS_FClose(fd);
		}
		FS_FClose(fs);
	}
	return cpy_size==fs_size;
}

void CopySongFromUDisk(void)
{
#ifdef __UPLOAD_CORE__

	FS_FILE *fs;

	int i;
#else

#ifdef __UPDATE_TASKFILE__
	FS_FILE *fs;
	u8 b=FALSE;
#endif

#ifdef __UPDATE_SONGFILE__
	int i;
#endif

#endif

#ifdef __UPDATE_SONGFILE__
	u8 b1=FALSE;
#ifndef __UPDATE_TASKFILE__
	u8 b=FALSE;
#endif
	FS_DIR *dirp;
	char sfn[255];	
	char dfn[255];
#endif


#ifdef __UPDATE_TASKFILE__
	extern void Dsp_UpdateTaskFile(void);
#endif
#ifdef __UPDATE_SONGFILE__
	extern void Dsp_UpdateSongFile(void);
#endif

#ifdef __UPLOAD_CORE__
	extern void Dsp_UpdateCore(void);
	char core[0x100000];

#ifdef 	_SM28800_
	fs=FS_FOpen("ud:\\UPLOAD-S\\SM28800.BIN","rb");
#endif
#ifdef	_SM28900_
	fs=FS_FOpen("ud:\\UPLOAD-S\\SM28900.BIN","rb");
#endif
#ifdef	_SM27800_
	fs=FS_FOpen("ud:\\UPLOAD-S\\SM27800.BIN","rb");
#endif
#ifdef	_SM23100_
	fs=FS_FOpen("ud:\\UPLOAD-S\\SM23100.BIN","rb");
#endif
	if(fs&&(fs->size<=0x100000)&&(fs->size>0x20000))
	{
		i= FS_FRead(core,1,fs->size,fs);
		if(i==fs->size)
		{
			Dsp_UpdateCore();

			kb_GetOKorESCKey();

			FS_FClose(fs);
			if(kb_KeyCode==VK_OK)
			{
				Putstr(30,32,"正在更新系统，请稍候！",22,1);
				Putstr(18,48,"更新完毕黑屏，请关电重开！",26,1);
				UpdateLCMData();
			

				while(SectorProg(0x50000, (unsigned short * )core, i));
				reboot();
			}
		}
	}
#endif

#ifdef __UPDATE_TASKFILE__
	fs=FS_FOpen("ud:\\task\\BOYIN.TGL","rb");
	if(fs)
	{
#ifdef _NEW_TASK_
		b=fs->size==TASK_DATA_SIZE;
#else
		b=fs->size==TASKGROUPLIST_SIZE;
#endif
		FS_FClose(fs);
	}
	if(b)
	{
		Dsp_UpdateTaskFile();
		kb_GetOKorESCKey();
		
		if(kb_KeyCode==VK_OK)
		{
			Putstr(30,32,"正在更新任务，请稍候！",22,1);
			UpdateLCMData();

			b= CopyFile("ud:\\task\\BOYIN.TGL",TASKFILENAME);

		}
	}
#endif	//#ifdef __UPDATE_TASKFILE__

#ifdef __UPDATE_SONGFILE__

	dirp= FS_OpenDir("ud:\\SONG\\");
	if(dirp)
	{
		b1=TRUE;
    	FS_CloseDir(dirp);
	}
	if(b1)
	{
		Dsp_UpdateSongFile();
		
		kb_GetOKorESCKey();

		if(kb_KeyCode==VK_OK)
		{
			Putstr(30,32,"正在复制文件，请稍候！",22,1);
			UpdateLCMData();

			memset(&view_audiolist,0,sizeof(view_audiolist));
			GetFileList("ud:\\SONG\\",&view_audiolist,0,0);
		
			for(i=0;i<view_audiolist.count;i++)
			{
				strcpy(sfn,view_audiolist.path[0]);
				strcat(sfn, view_audiolist.filelist[i]);
			
				strcpy(dfn, EXTSONGPATH);

				strcat(dfn, view_audiolist.filelist[i]);
			
				CopyFile(sfn,dfn);
			}
		}
	}
	if(b||b1)
	{
//	 	flash_FlushCache(0,0xFF);
#if	(NANDFLASH_PARTITION_MAXUNIT==1)
//	 	flash_FlushCache(1,0xFF);
#endif
	 	if(b)
		{
			if (ReadTaskFile()==TRUE)
				LoadTodayTask();
		}
		if(b1)
			GetMP3FileList();
	}
#endif //#ifdef __UPDATE_SONGFILE__
	Lcmclr();
	Dsp_MM_Desktop();
}





