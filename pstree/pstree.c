#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <assert.h>
/* 进程信息 */
struct file_info{
  	int pid;//进程号
	int ppid;//父进程号
	char name[1024];//进程名字
};
/* 多叉树 */
typedef struct tree{
	int pid;
	char name[1024];
	struct tree *Child[128];	
}TreeNode,*pTreeNode;

/* 显示进程树 */
void show_tree(TreeNode*node,int level){
	if(node==NULL)return;
	printf("-%s\n",node->name);
	printf("%*s",level*9," ");
	printf("|");
	for(int i=0;i<128;++i){
		show_tree(node->Child[i],level+1);
	}
}

/* 构建进程树 */
void create_tree(struct file_info *fi,pTreeNode node){
	int sons[2048];
	int j=0;
	for(int i=0;i<4096;++i){
		/* 寻找父进程为node的进程，将其存储到数组中 */
		if(fi[i].ppid==node->pid){
			j++;
			sons[j]=i;
		}
	}
	/* 如果找到的进程数组不为空，将其赋值到父进程中，否则返回 */
	if(j==0)
		return;
	else{
		int m=0;
		while(j>0){
			pTreeNode newNode=(pTreeNode)malloc(sizeof(TreeNode));
			newNode->pid=fi[sons[j]].pid;
			strcpy(newNode->name,fi[sons[j]].name);
			node->Child[m]=newNode;
			create_tree(fi,newNode);
			j--;	
			m++;
		}
	}
}

/* 获得父进程id和进程名字 */
void get_fpid(struct dirent*entry,struct file_info *i){
	char path[64]="/proc/";
	strcat(path,entry->d_name);
	DIR*dir=opendir(path);
	struct dirent*subentry;
	int p_pid=0;char n_ame[1024];
	while((subentry=readdir(dir))!=NULL){
		if(strcmp(subentry->d_name,"stat")!=0)
			continue;
		strcat(path,"/");
		strcat(path,subentry->d_name);
		FILE*fp=fopen(path,"r");
		if(fp){
			int ret=fscanf(fp,"%*d (%s %*s %d",n_ame,&p_pid);
			//if(ret!=0)
				//perror("perror error");	
		}else
			perror("fopen error");
		fclose(fp);
	}
	i->ppid=p_pid;
	n_ame[strlen(n_ame)-1]='\0';//去除进程名称的最后一个括号
	strcpy(i->name,n_ame);
}

int main(int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  assert(!argv[argc]);

  DIR*dir;
  struct dirent*entry;

  int i=0;

  struct file_info file[4096];
  int count=0;//记录当前进程数
  char search="/proc";
  //统计每个进程
  if((dir=opendir("/proc"))==NULL)
	  perror("opendir() error");
  else{
  	while((entry=readdir(dir))!=NULL){
		if(entry->d_name[0]>'0'&&entry->d_name[0]<='9'){
			//记录进程id
			file[count].pid=atoi(entry->d_name);
			//记录父进程id及进程名称
			get_fpid(entry,&file[count]);

	count++;
		}
	}
  }

  //构建进程树
  TreeNode root;
  for(i=0;i<4096;++i){
  	if(file[i].pid==1){
		root.pid=file[i].pid;
		strcpy(root.name,file[i].name);
		break;
	}
  }
  create_tree(file,&root);

  closedir(dir);

  //i=0;
  //while(file[i].pid!=0){
  //	printf("pid=%d,ppid=%d,name=%s\n",file[i].pid,file[i].ppid,file[i].name);
  //	i++;
  //}

  /* 显示进程树*/
  show_tree(&root,0);

  return 0;
}