//判定时间翻了两番了。。
//把文件上传到github上面了，测试一下会怎么样更新

#include <windows.h>
#include <stdio.h>//standard io
#include<conio.h>//_getch();
//为了实现随机
#include<stdlib.h>//srand(),rand()
#include<time.h>//time()

//角色结构，有生命值，x坐标，y坐标
struct character
{
	char alive = 0;//是否活着
	int life;//生命值
	int x;
	int y;
};

struct anAmmo
{
	char isNew = 1;
	char isBroken = 0;
	char isRunning = 0;//正在滑动轨道上
	char location = 0;//零表示未出现，1最右，7最左，8已经进入发射腔，9已经发射
	char ID[200];
};

//待用变量,应该再一开始先声明有什么变量，再主程序里再给他们赋值
int score;
char imageCache[200][200];//图像缓存，用来储存将要输出的内容 [y][x]
struct anAmmo ammo[200];
char ammoNumber;
char message[50][80];
int successfulLoadingTimes;
int pilotIsAlive;
int playerCanControlFlight;
int gameStage;//游戏阶段：1子机对战，2Boss战
char thisBeamHasHurt = 0;
int gameStageTimmer;


struct character player;
struct character playerBullet;
struct character enermyChild;
struct character boss;
struct character bossBullet1;
struct character bossBullet2;
struct character bossBeam;

//===================================================================一层 基础工具=========================================================================================================================================================================================================

//设置光标的位置
void setCursorAt(int x, int y)//把光标设置在指定坐标
{
	COORD p;       //COORD是个坐标结构
	p.X = x;      //坐标
	p.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), p);      //设置控制台光标的到指定坐标
}
//隐藏光标
void hideCursor(void)
{
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
	cursorInfo.bVisible = false;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}
//方向键的状态：0无按下，1上，2下，3左，4右
int stateOfArrowKey(void)
{
	if (GetKeyState(0x57) < 0) return 1;
	if (GetKeyState(0x53) < 0) return 2;
	if (GetKeyState(0x41) < 0) return 3;
	if (GetKeyState(0x44) < 0) return 4;
	return 0;
}
//空格键状态:1按下，0松开
int stateOfSpace(void)
{
	if (GetKeyState(VK_SPACE) < 0) return 1;
	else return 0;
}
//前一帧按下了什么按键：0没有按键,59;,108l,107k,106j
int keyOfPreviousFrame(void)
{
	int i = 0;
	while (_kbhit())
	{
		i =  _getch();
	}
	return i;
}
//文字变色
void turnGreen()
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon,0x2f);
}
void turnRed()
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, 0x4f);
}
void turnWhite()
{
	HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hCon, 0x07);
}
//清空图像缓存区
void clearImageCache(void)
{
	for (int y = 0; y <= 50; y++) 
	{
		for (int x = 0; x <= 100; x++)
		{
			imageCache[y][x] = ' ';
		}
		imageCache[y][101] = '\0';
	}
}
//将缓存区内的图像打印到屏幕上
void displayImageCache(void)
{
	setCursorAt(0, 0);
	for (int y = 0; y <= 45; y++)
	{
		
		printf("%s", imageCache[y]);
		printf("\n");
	}
}
//把新的内容写入图像缓存区
void writeImageCache(int x, int y, const char text[52])
{
	int nowWriting = 0;//正在操作的text位置
	while (text[nowWriting] != '\0')
	{
		imageCache[y][x + nowWriting] = text[nowWriting];
		nowWriting++;
	}
}
//清除消息
void clearMessage()
{
	for (int i = 0; i <= 49; i++)
	{
		message[i][0] = '\0';
	}
}
void newMessage(const char* text)
{
	for (int i = 49; i >= 0; i--)//所有的字符串序号加一
	{
		int n = 0;
		while (message[i - 1][n] != '\0')//循环将前一字符串的字符复制到后一字符串的同一位置，直到遇到\0
		{
			message[i][n] = message[i - 1][n];
			n++;
		}
		message[i][n] = '\0';
	}
	//下面把输入的字符串放到message[0]
	int n = 0;
	while (text[n] != '\0')//循环将输入的字符复制到第一字符串的同一位置，直到遇到\0
	{
		message[0][n] = text[n];
		n++;
	}
	message[0][n] = '\0';
}
//===================================================================二层 基础工具=========================================================================================================================================================================================================


int line = 2;//当前行数
//调试用，把信息显示在info栏
void putInformation(const char message[50])
{
	setCursorAt(44, line);
	printf(message);
	line++;
}

//===================================================================三层1 视觉面板=========================================================================================================================================================================================================

//绘制血量界面
void drawPlayerLifeInterface(void)
{
	writeImageCache(1, 0, "本机生命：");
	for (int i = 1; i <= 5; i++)
	{
		if (player.life >= i)
		{
			writeImageCache(10+i, 0, "0");
		}
		else {
			writeImageCache(10 + i, 0, "<");
		}
	}
}
void drawBossLifeInterface(void)
{

	writeImageCache(20, 0, "BOSS生命：");
	for (int i = 1; i <= 5; i++)
	{
		if (boss.life >= i)
		{
			writeImageCache(29 + i, 0, "0");
		}
		else {
			writeImageCache(29 + i, 0, "<");
		}
	}
}
//绘制基本界面：游戏边框
void drawBasicInterface(void)
{
	//绘制第一排的白板，boss的部分身体有时候出现在那里，需要遮盖
	writeImageCache(1, 0, "                                        ");
	//绘制边框
	writeImageCache(1, 1, " -------------------------------------- ");
	for (int i = 2; i != 30; i++)
	{
		writeImageCache(1, i, "|");
		writeImageCache(40, i, "|");
	}
	writeImageCache(1, 30, " -------------------------------------- ");
}
//绘制人物下面的白板
void drawBackground()
{
	
	for (int a = 0; a <= 39; a++)
	{
		writeImageCache(2, a, "                                      ");
	}
}
//绘制分数界面
void drawScoreInterface(void)
{
	setCursorAt(15, 41);
	printf("SCORE:%d", score);
	writeImageCache(1, 0, "                                        ");
}
//绘制玩家飞机
void drawPlayer(void)
{
	writeImageCache(player.x, player.y - 2, "A");
	writeImageCache(player.x, player.y - 1, "H");
	writeImageCache(player.x - 1, player.y, "/P\\ ");
	writeImageCache(player.x - 3, player.y + 1, "-=qop=-");
}
//绘制玩家子弹
void drawPlayerBullet(void)
{
	writeImageCache(playerBullet.x, playerBullet.y, "|");
}
//绘制敌方子机
void drawEnermyChild(void)
{
	writeImageCache(enermyChild.x, enermyChild.y, "T");
}
//绘制BOSS飞机
void drawBoss(void)
{
	writeImageCache(boss.x, boss.y + 2, "V");
	writeImageCache(boss.x, boss.y + 1, "H");
	writeImageCache(boss.x - 1, boss.y, "\\B/ ");
	writeImageCache(boss.x - 3, boss.y - 1, "-=dob=-");
}
//绘制BOSS子弹
void drawBossBullet1(void)
{
	writeImageCache(bossBullet1.x, bossBullet1.y, "|");
}
void drawBossBullet2(void)
{
	writeImageCache(bossBullet2.x, bossBullet2.y, "|");
}
//绘制BOSS激光
void drawBossBeam(void)
{
	for (int i = bossBeam.y + 3; i <= 29; i++)
	{
		writeImageCache(bossBeam.x, i, "!");
	}
}

//===================================================================三层2 文本面板=========================================================================================================================================================================================================


//绘制静态界面A：下面的标题，静止画等，A表示这是在玩家得到移动授权之前的界面
void drawStaticInterfaceA(void)
{
	writeImageCache(1, 32, "通信对象：");
	writeImageCache(3, 33, "公网/鹭九/0292");
	writeImageCache(1, 34, "延迟：");
	writeImageCache(3, 35, "14ms");
	writeImageCache(1, 36, "中继：");
	writeImageCache(3, 37, "无");
}
void drawStaticInterfaceB(void)
{
	writeImageCache(1, 32, "通信对象：");
	writeImageCache(3, 33, "鹭九0292/自动巡航系统");
	writeImageCache(1, 34, "延迟：");
	writeImageCache(3, 35, "29ms");
	writeImageCache(1, 36, "中继：");
	writeImageCache(3, 37, "1次");
	writeImageCache(1, 38, "航向校正");
	writeImageCache(3, 39, "前W     S后");
	writeImageCache(3, 40, "左A     D右");
	writeImageCache(7, 39, "-+-");
	writeImageCache(7, 40, "-+-");
	if (GetKeyState(0x57) < 0)		writeImageCache(7, 39, "+--");
	if (GetKeyState(0x53) < 0)	writeImageCache(7, 39, "--+");
	if (GetKeyState(0x41) < 0)	writeImageCache(7, 40, "+--");
	if (GetKeyState(0x44) < 0)	writeImageCache(7, 40, "--+");
}
//绘制装弹界面
void drawLoaderInterface(void)
{
	//绘制边框和标题
	writeImageCache(25, 32, " -  弹识别码：");
	writeImageCache(25, 33, "|E|");
	writeImageCache(25, 34, "|M|");
	writeImageCache(25, 35, "|P|");
	writeImageCache(25, 36, "|T|");
	writeImageCache(25, 37, "|Y| 状态：");
	writeImageCache(25, 38, "| |");
	writeImageCache(25, 39, "| |");
	writeImageCache(25, 41, "| ");
	writeImageCache(25, 42, "| ");
	writeImageCache(25, 43, "|- ");
	writeImageCache(25, 45, " J K       L :");

}
void createANewAmmo(void)
{
	ammoNumber++;
	
		for (int i = 1; i <= 100; i++)
		{
			if (rand() % 4 == 0)
				ammo[ammoNumber].ID[i] = (rand() % 10) + 48;
			else
				ammo[ammoNumber].ID[i] = (rand() % 26) + 65;
		}
}
//在识别码一栏，填写子弹识别码
void IDBar_fillWithAmmoID()
{

	//用随机字符串里的字符填写四行子弹识别码
	char IDline1[11] = { ammo[ammoNumber].ID[1],ammo[ammoNumber].ID[2],ammo[ammoNumber].ID[3],ammo[ammoNumber].ID[4] ,ammo[ammoNumber].ID[5] ,ammo[ammoNumber].ID[6] ,ammo[ammoNumber].ID[7],'-',ammo[ammoNumber].ID[8] ,'\0' };
	char IDline2[11] = { ammo[ammoNumber].ID[9],ammo[ammoNumber].ID[10],ammo[ammoNumber].ID[11],ammo[ammoNumber].ID[12] ,ammo[ammoNumber].ID[13] ,ammo[ammoNumber].ID[14],'-' ,ammo[ammoNumber].ID[15],ammo[ammoNumber].ID[16] ,'\0' };
	char IDline3[11] = { ammo[ammoNumber].ID[17],ammo[ammoNumber].ID[18],ammo[ammoNumber].ID[19],ammo[ammoNumber].ID[20] ,ammo[ammoNumber].ID[21],'-' ,ammo[ammoNumber].ID[22] ,ammo[ammoNumber].ID[23],ammo[ammoNumber].ID[24] ,'\0' };
	char IDline4[11] = { ammo[ammoNumber].ID[25],ammo[ammoNumber].ID[26],ammo[ammoNumber].ID[27],ammo[ammoNumber].ID[28],' ' ,' ' ,' ',' ',' ','\0' };
	//输出子弹识别码
	writeImageCache(31, 33, IDline1);
	writeImageCache(31, 34, IDline2);
	writeImageCache(31, 35, IDline3);
	writeImageCache(31, 36, IDline4);

	ammo[ammoNumber].isNew = 0;//已经读过子弹代码，不再读这个子弹的了
}
//在识别码一栏，填写NO_INFO
void IDBar_noInfo()
{
	writeImageCache(31, 33, "NO-INFO  ");
	writeImageCache(31, 34, "         ");
	writeImageCache(31, 35, "         ");
	writeImageCache(31, 36, "         ");
}
//绘制装弹器状态，使用了printf写法，要放在drawall的最最最后
void drawLoaderState()
{
	setCursorAt(31,38);
	if (ammo[ammoNumber].location != 8)
	{
		printf("发射仓空");
	}
	else if (ammo[ammoNumber].isBroken)
	{
		turnRed();
		printf("自检报错");
		turnWhite();
	}
	else if (ammo[ammoNumber].isBroken == 0)
	{
		turnGreen();
		printf("等待发射");
		turnWhite();
	}
}
//绘制装弹器上的子弹提示标
void drawAmmoIcon()
{
	switch (ammo[ammoNumber].location)
	{
	case 9:

		break;
	case 8:
	{
		writeImageCache(26, 33, "A");
		writeImageCache(26, 34, "H");
		writeImageCache(26, 35, "H");
		writeImageCache(26, 36, "H");
		writeImageCache(26, 37, "H");
		writeImageCache(26, 38, "H");
		writeImageCache(26, 39, "H");
	}
	break;
	case 7:
	{
		writeImageCache(26, 41, "A");
		writeImageCache(26, 42, "H");
	}
	break;
	case 6:
	{
		writeImageCache(28, 42, "A");
		writeImageCache(28, 43, "H");
	}
	break;
	case 5:
	{
		writeImageCache(30, 42, "A");
		writeImageCache(30, 43, "H");
	}
	break;
	case 4:
	{
		writeImageCache(32, 42, "A");
		writeImageCache(32, 43, "H");
	}
	break;
	case 3:
	{
		writeImageCache(34, 42, "A");
		writeImageCache(34, 43, "H");
	}
	break;
	case 2:
	{
		writeImageCache(36, 42, "A");
		writeImageCache(36, 43, "H");
	}
	break;
	case 1:
	{
		writeImageCache(38, 42, "A");
		writeImageCache(38, 43, "H");
	}
	break;
	case 0:
	break;
	}
}
//绘制通讯界面
void drawMessages()
{
	//绘制信息栏
	for (int i = 0; i <= 46; i++)
	{
		writeImageCache(41, i, "|");
	}
	writeImageCache(42, 44, "-----------------------------------------------------------");
	writeImageCache(43, 45, "通讯");
	//绘制信息
	for (int y = 43; y >= 1; y--)
	{
		int messageNumber = 43 - y;
		writeImageCache(42, y, message[messageNumber]);
	}
}


//===================================================================四层 高级工具、结算界面=========================================================================================================================================================================================================
void drawAll(void)
{
	//盘内绘图：飞机、子弹、敌人
	{
		clearImageCache();
		if (enermyChild.alive) drawEnermyChild();
		if (bossBullet1.alive) drawBossBullet1();
		if (bossBullet2.alive) drawBossBullet2();
		if (playerBullet.alive) drawPlayerBullet();
		if (bossBeam.alive) drawBossBeam();
		drawPlayer();
		if (boss.alive) drawBoss();
		drawBasicInterface();
		drawPlayerLifeInterface();
		if(boss.alive)	drawBossLifeInterface();
	}
	//盘外绘图：面板、通讯
	{
		if (!playerCanControlFlight)	drawStaticInterfaceA();
		if (playerCanControlFlight )	drawStaticInterfaceB();
		drawLoaderInterface();
		if (ammo[ammoNumber].location == 8)
			IDBar_fillWithAmmoID();
		if (ammo[ammoNumber].location != 8)	
			IDBar_noInfo();
		drawAmmoIcon();
		drawMessages();
	}
	displayImageCache();
	drawLoaderState();
}	
void creatEnermyChildAt(int x, int y)
{
	enermyChild.alive = 1;
	enermyChild.x = x;
	enermyChild.y = y;
}
void gameOver(void)
{
	system("cls");
	fflush(stdin);
	printf("你所乘的战机被击落，游戏结束了。\n");
	printf("不过没有关系，稍后进入关卡选择界面的时候，你可以从储存点重新开始。\n");
	printf("也可以跳过这个阶段，玩一玩后面的内容。\n\n");
	Sleep(1000);
	printf("按任意键继续。\n");
	_getch();
	while (getchar() != '\n');
}
void gameClear(void)
{
	system("cls");
	printf("你顺利完成了任务！\n");
	printf("战机已经开始自动返航，好好休息一下吧。\n");
	printf("回到基地，说不定可以和今天的指挥员见个面。\n");
	Sleep(1000);
	printf("按任意键结束游戏。\n");
	fflush(stdin);
	_getch();
	exit(0);
}
void selectStage(void) 
{
	system("cls");
	printf("这里是游戏场景选择界面。\n\n");
	printf("S：从头开始游戏\n");
	printf("R：从上次游戏的储存点开始。储存点的位置是：场景%d\n\n",gameStage);
	printf("也可以从特定场景开始：\n");
	printf("  1：装弹教学\n");
	printf("  2：和驾驶员并肩作战\n");
	printf("  3：移动教学\n");
	printf("  4：独自前进\n");
	printf("  5：BOSS战\n\n");
	Sleep(1000);
	printf("请选择并输入代码：");
	char a = getchar();
	if (a == 'S')		gameStage = 1;
	if (a == 'R');
	if (a == '1')		gameStage = 1;
	if (a == '2')		gameStage = 2;
	if (a == '3')		gameStage = 3;
	if (a == '4')		gameStage = 4;
	if (a == '5')		gameStage = 5;
	
}

//===================================================================顶层 行动函数+碰撞检测==========================================================================================================================================================================================================
void playerAct()
{
	if (pilotIsAlive)//飞行员控制
	{
		if (player.y != 27) player.y++;
		if (player.y - enermyChild.y >= 5)//要是玩家和敌人离得很远，就对准敌人
		{
			if (player.x - enermyChild.x > 0)	player.x--;
			if (player.x - enermyChild.x < 0)	player.x++;
		}
		else//要是玩家和敌人离得很近，就远离它
		{
			if (enermyChild.x >= 20) player.x--;//要是敌人在屏幕右侧，飞行员就向左开
			if (enermyChild.x < 20) player.x++;
		}

	}
	if (playerCanControlFlight)//玩家控制
	{
		//1.玩家移动
		if (GetKeyState(0x57) < 0) player.y--;
		if (GetKeyState(0x53) < 0) player.y++;
		if (GetKeyState(0x41) < 0) player.x--;
		if (GetKeyState(0x44) < 0) player.x++;
	}
	//2.检查移动后是否触碰边界，要是会碰到就不要动了
	if (player.x - 3 == 1) player.x++;
	if (player.x + 3 == 40) player.x--;
	if (player.y - 2 == 1) player.y++;
	if (player.y +1 == 30) player.y--;
	drawPlayer();
}
void playerBulletAct()
{
	playerBullet.y--;
	if (playerBullet.y == 1) playerBullet.alive = 0;
	drawPlayerBullet();
}
void enermyChildAct()
{
	enermyChild.y++;
	if (enermyChild.y == 30) enermyChild.alive = 0;
	drawEnermyChild();
}
int roundsSeeingPlayer;//持续看见玩家的回合数
void bossAct() 
{
	//攻击
	if (boss.x == player.x) roundsSeeingPlayer++;
	else roundsSeeingPlayer = 0;
	if (roundsSeeingPlayer == 5)//持续五回合看见玩家就开火,并且计数清零
	{
		bossBeam.alive = 1;
		bossBeam.x = boss.x;
		bossBeam.y = boss.y;

		roundsSeeingPlayer = 0;
	}
	if (gameStageTimmer % 7 == 0)//对于这个act函数来说，timmer每次增加五十，所以若干次之后就能被7整除
	{
		bossBullet1.alive = 1;
		bossBullet1.x = boss.x - 2;
		bossBullet1.y = boss.y + 2;
		bossBullet2.alive = 1;
		bossBullet2.x = boss.x + 2;
		bossBullet2.y = boss.y + 2;
	}
	//移动
	if (boss.y != 7) boss.y++;
	if (boss.x < player.x) boss.x++;
	if (boss.x > player.x) boss.x--;
	drawBoss();
}
void bossBullet1Act()
{
	//移动
	bossBullet1.y++;
	if (bossBullet1.y == 30) bossBullet1.alive = 0;
	drawBossBullet1();
}
void bossBullet2Act()
{
	//移动
	bossBullet2.y++;
	if (bossBullet2.y == 30) bossBullet2.alive = 0;
	drawBossBullet2();
}
void bossBeamAct()
{
	bossBeam.alive = 0;
	thisBeamHasHurt = 0;
}
int timePush = 0;//推出子弹用的时间
int cycleTimmer = 0;//记录这一帧到达了一个周期的多少时间
char isPushing = 0;
char isRunning = 0;
void loaderAct()
{
	if (ammo[ammoNumber].location == 1 && isPushing == 0)//如果在起始位置，还没开始推，就等待推
	{
		if (keyOfPreviousFrame() == 59) 
		{
			isPushing = 1;
		}
	}
	if (isPushing == 1)// 如果正在推，就保持计时
	{
		timePush++;
		if (timePush >= 400)//要是等待的时间太久，就不等了
		{
			isPushing = 0;
			timePush = 0;
		}
	}
	if (isPushing == 1)//如果正在推，就等待推动结束
	{
		if (keyOfPreviousFrame() == 108)//当推动结束，就开始运动，停止计时
		{
			isPushing = 0;
			isRunning = 1;
			ammo[ammoNumber].location = 2;

		}
	}
	if (isRunning && ammo[ammoNumber].location <= 5)//如果正在运动，还没有到达接收器，就等到达计时周期
	{
		cycleTimmer++;
		if (cycleTimmer == timePush)//如果到达了计时周期，就子弹运动，并且开始新一轮计时
		{
			ammo[ammoNumber].location++;
			cycleTimmer = 0;
		}
	}
	if (ammo[ammoNumber].location == 6)//如果到了第一个接收器，就计时，并等待按键
	{
		cycleTimmer++;
		if (keyOfPreviousFrame() == 107)//如果按下按键，就判断弹药是否损坏，并把子弹往下推一格，并还原计时器。这里可以调整宽容度
		{
			if (cycleTimmer - timePush >= 15 || cycleTimmer - timePush <= -15)//如果没有在指定时间内按键，就损坏弹药
			{
				ammo[ammoNumber].isBroken = 1;
			}
			ammo[ammoNumber].location++;
			cycleTimmer = 0;
		}
	}
	if (ammo[ammoNumber].location == 7)//如果到了第二个接收器，就计时，并等待按键
	{
		cycleTimmer++;
		if (keyOfPreviousFrame() == 106)//如果按下按键，就判断弹药是否损坏，并把子弹往下推一格
		{
			if (cycleTimmer - timePush >= 15 || cycleTimmer - timePush <= -15)//如果没有在指定时间内按键，就损坏弹药
			{
				ammo[ammoNumber].isBroken = 1;
			}
			ammo[ammoNumber].location++;
		}
	}
	if (ammo[ammoNumber].location == 8)//如果进入发射腔，就等待发射或者清空
	{
		if (keyOfPreviousFrame() == 32)//如果按下按键，就判断弹药是否损坏，如果没有损坏就发射，然后重置发射器
		{
			if (ammo[ammoNumber].isBroken)//如果损坏，就清空弹舱，生成一个新炮弹
			{
			}
			else
			{
				playerBullet.alive = 1;
				playerBullet.x = player.x;
				playerBullet.y = player.y;
				successfulLoadingTimes++;
			}
			//下面重置发射器
			createANewAmmo();
			ammo[ammoNumber].location = 1;
			cycleTimmer = 0;
			timePush = 0;
			isPushing = 0;
			isRunning = 0;
		}
	}
}
//检查碰撞，然后根据碰撞情况产生效果
void checkContact()
{
	//如果玩家和敌方子机碰撞，那么子机死亡，玩家扣血
	if (//注意这里条件有三行
			enermyChild.x == player.x && //玩家和敌机的在横向上重合
			(-1<=enermyChild.y - player.y && enermyChild.y-player.y<=1)&&//玩家和敌机的纵向距离小于等于1
			enermyChild.alive//敌机还活着
		)
	{
		enermyChild.alive = 0;
		player.life--;
		drawPlayerLifeInterface();
		drawEnermyChild();
	}

	//如果玩家和敌方子弹1碰撞，那么子弹死亡，玩家扣血
	if (//条件不做解释，参考玩家和敌方子机碰撞的情况
			bossBullet1.x == player.x &&
			(-1 <= bossBullet1.y - player.y && bossBullet1.y - player.y <= 1)&&
			bossBullet1.alive
		)
	{
		bossBullet1.alive = 0;
		player.life--;
		drawPlayerLifeInterface();
		drawBossBullet1();
	}

	//如果玩家和敌方子弹2碰撞，那么子弹死亡，玩家扣血
	if (
			bossBullet2.x == player.x &&
			(-1 <= bossBullet2.y - player.y && bossBullet2.y - player.y <= 1)&&//玩家和敌机的距离小于等于1
			bossBullet2.alive
		)
	{
		bossBullet2.alive = 0;
		player.life--;
		drawPlayerLifeInterface();
		drawBossBullet2();
	}

	//如果玩家子弹和敌方子机碰撞，那么子机死亡,玩家子弹死亡，加一分
	if (
			enermyChild.x == playerBullet.x &&
			(-1 <= enermyChild.y - playerBullet.y && enermyChild.y - playerBullet.y <= 1)&&//敌机与子弹的距离小于等于1
			enermyChild.alive
		)
		
	{
		enermyChild.alive = 0;
		playerBullet.alive = 0;
		score++;
		drawPlayerBullet();
		drawScoreInterface();
		drawEnermyChild();
	}

	//要是玩家和boss激光纵向重合并且玩家没扣过血，就扣血
	if (player.x == bossBeam.x && bossBeam.alive && thisBeamHasHurt == 0)
	{
		player.life-=2;
		thisBeamHasHurt = 1;
		drawPlayerLifeInterface();
	}

	//要是玩家子弹和boss碰撞，boss扣血，玩家子弹死亡
	if (
			boss.x == playerBullet.x &&
			(-1 <= boss.y - playerBullet.y && boss.y - playerBullet.y <= 1)&&
			playerBullet.alive
		)

	{
		boss.life--;
		playerBullet.alive = 0;
		drawBossLifeInterface();
		drawPlayerBullet();
	}
}
//检查游戏情况，然后根据情况更新游戏状态
int flag;
int checkGameStage()//0游戏中，1胜利，2死亡
{
	gameStageTimmer++;
	//装弹教学
	if (gameStage == 1)
	{
		//如果玩家血量被打到0，那么就进入Game Over界面
		if (player.life <= 0)
		{
			gameOver();
			return 2;
		}
		if (flag == 1 && gameStageTimmer == 300)
		{
			pilotIsAlive = 0;
			playerCanControlFlight = 0;
			newMessage("公网：驾驶员对你很不满意。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 2 && gameStageTimmer == 300)
		{
			newMessage("公网：你是海军，船上的装弹器和飞机上的不一样。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 3 && gameStageTimmer == 400)
		{
			newMessage("公网：所以我理解你。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 4 && gameStageTimmer == 450)
		{
			newMessage("公网：但我们人手短缺。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 5 && gameStageTimmer == 550)
		{
			newMessage("公网：要操作装填器，你只需要用五个按键。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 6 && gameStageTimmer == 350)
		{
			newMessage("公网：显然你的工作会很简单。你看，都在迎敌途中了，我才开始");
			newMessage("给你做培训。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 7 && gameStageTimmer == 700)
		{
			newMessage("公网：顺次按下“；L”，主推柱会按照你的速度把弹药推走。");
			flag++;
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 9 && gameStageTimmer == 600)
		{
			newMessage("公网：通道上等距离安装了光电门，发亮的区域表明弹药正在上面");
			newMessage("运动。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 10 && gameStageTimmer == 900)
		{
			newMessage("公网：接下来是难点，KJ按键可以操作接收控制器。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 11 && gameStageTimmer == 400)
		{
			newMessage("公网：当弹药到达分歧控制点K时，按下K键，侧推柱会给弹药侧向"); 
			newMessage("推力，使它对准装弹槽。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 12 && gameStageTimmer == 700)
		{
			newMessage("公网：最后，弹药到达装弹槽J时，按下J键，装填机就会顺势把弹");
			newMessage("药送入发射腔。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 13 && gameStageTimmer == 700)
		{
			newMessage("公网：然后按下空格就可以释放你刚刚装入的弹药了。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 14 && gameStageTimmer == 400)
		{
			newMessage("公网：当然，如果你的装填损坏了弹药，发射器是不会给弹药点火");
			newMessage("的。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 15 && gameStageTimmer == 700)
		{
			newMessage("公网：简而言之，你要保证弹药匀速运动，它到哪里，你就按哪个");
			newMessage("键。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 16 && gameStageTimmer == 700)//根据之前是否装填成功，有不同的信息
		{
			if (successfulLoadingTimes != 0)
			{
				newMessage("公网：看来在我教你的时候你已经成功发射过一次了。不愧是海军");
				newMessage("装弹手。");
				flag++;
				flag++;//跳到flag18
				gameStageTimmer = 0;
			}
			if (successfulLoadingTimes == 0)
			{
				newMessage("公网：接下来你自己试一下。");
				flag++;//跳到flag17
				gameStageTimmer = 0;
			}
		}
		if (flag == 17 && gameStageTimmer >= 500 && successfulLoadingTimes!=0)
		{
			newMessage("公网：就是这样。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 18 && gameStageTimmer == 600)
		{
			newMessage("公网：注意，机动飞行时不要装弹。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 19 && gameStageTimmer == 300)
		{
			newMessage("公网：如果你愿意知道更多的话，其实职业装弹员大多数时候都在");
			newMessage("练习机动飞行下的装弹。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 20 && gameStageTimmer == 500)
		{
			newMessage("公网：而不是匀速情况下的。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 21 && gameStageTimmer == 300)
		{
			newMessage("公网：因此你的驾驶员不得不迁就你，把飞机开得平稳一点。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 22 && gameStageTimmer == 400)
		{
			newMessage("公网：所以他会不高兴。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 23 && gameStageTimmer == 400)
		{
			newMessage("公网：你先练习。接敌之前我会通知你。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 24 && successfulLoadingTimes == 5) 
		{
			//进入第二阶段
			flag = 1;
			gameStage = 2;
			gameStageTimmer = 0;
		}

	}
	//和飞行员并肩作战
	if (gameStage == 2)
	{
		//如果玩家血量被打到0，那么就进入Game Over界面
		if (player.life <= 0)
		{
			gameOver();
			return 2;
		}
		if (flag == 1 && gameStageTimmer == 1000)
		{
			pilotIsAlive = 1;
			playerCanControlFlight = 0;
			newMessage("公网：准备接敌");
			playerCanControlFlight = 0;
			pilotIsAlive = 1;
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 2 && score<5 && enermyChild.alive == 0) creatEnermyChildAt((rand() % 32) + 5, 1);
		if (flag == 2 && score == 5)
		{
			gameStage++;
			flag = 1;
			gameStageTimmer = 0;
		}
	}
	//驾驶教学
	if (gameStage == 3)
	{
		//如果玩家血量被打到0，那么就进入Game Over界面
		if (player.life <= 0)
		{
			gameOver();
			return 2;
		}
		if (enermyChild.alive == 0) creatEnermyChildAt((rand() % 32) + 5, 1);//一直刷新敌人飞机
		if (flag == 1 && gameStageTimmer == 1000)
		{
			playerCanControlFlight = 0;
			pilotIsAlive = 0;
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 2 && gameStageTimmer == 1000)
		{
			newMessage("公网：不好的消息。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 3 && gameStageTimmer == 200)
		{
			newMessage("公网：驾驶员失联了。");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 4 && gameStageTimmer == 150)
		{
			newMessage("公网：我给你接通偏航校准系统，你就能控制飞机");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 5 && gameStageTimmer == 250)
		{
			newMessage("公网：但我们的通讯会断开");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 6 && gameStageTimmer == 300)
		{
			newMessage("公网：祝你顺利完成任务，我的朋友");
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 7 && gameStageTimmer == 400)
		{
			newMessage("#错误 找不到通讯接口(0x24C64)");
			playerCanControlFlight = 1;
			flag = 1;
			gameStage = 4;
			gameStageTimmer = 0;
		}
	}
	//独自前进
	if (gameStage == 4)
	{
		//如果玩家血量被打到0，那么就进入Game Over界面
		if (player.life <= 0)
		{
			gameOver();
			return 2;
		}
		if (enermyChild.alive == 0 && score <= 10) creatEnermyChildAt((rand() % 32) + 5, 1);//杀死十个敌人前刷新敌人飞机
		if (flag == 1)
		{
			playerCanControlFlight = 1;
			score = 5;
			pilotIsAlive = 0;
			flag++;
			gameStageTimmer = 0;
		}
		if (flag == 2 && score == 10)
		{
			playerCanControlFlight = 1;
			pilotIsAlive = 0;
			gameStage++;
			flag = 1;
			gameStageTimmer = 0;
		}
	}
	//BOSS战
	if (gameStage == 5)
	{
		playerCanControlFlight = 1;
		pilotIsAlive = 0;
		//如果玩家血量被打到0，那么就进入Game Over界面
		if (player.life <= 0)
		{
			gameOver();
			return 2;
		}
		if (boss.life == 0)
		{
			gameClear();
		}
		//如果玩家血量被打到0，那么就进入Game Over界面
		if (player.life <= 0) gameOver();
		//如果没有产生BOSS,就产生BOSS
		if (boss.alive == 0)
		{
			boss.life = 5;
			boss.alive = 1;
			boss.x = 20;
			boss.y = 1;
		}
	}

	return 0;
	/*
	//当玩家分数达到6时，进入第二阶段
	if (score == 6) gameStage = 2;
	//在第一阶段中，如果敌人子机死了，那么就再生成一个
	if (gameStage == 1 && enermyChild.alive == 0) creatEnermyChildAt((rand() % 32) + 5, 1);
	//再第二阶段中，要是还没有生成boss，那就生成一个
	if (gameStage == 2 && boss.alive == 0)
	{
		boss.life = 10;
		boss.alive = 1;
		boss.x = 20;
		boss.y = 1;
	}
	//在第二阶段中，如果boss血量被打到0，那么就进入Game Clear界面
	if (boss.life == 0) gameClear();
	//如果玩家血量被打到0，那么就进入Game Over界面
	if (player.life <= 0) gameOver();
	*/
}
//===================================================================计时器结构，将行动函数填入对应周期的大括号中======================================================================================================================================
void runPerFrame(void)
{
	loaderAct();
}
void runPer5Frame(void)
{
	if (playerBullet.alive) playerBulletAct();
	if (bossBullet1.alive) bossBullet1Act();
	if (bossBullet2.alive) bossBullet2Act();
}
void runPer10Frame(void)
{
	playerAct();
	if (enermyChild.alive == 1) enermyChildAct();
}
void runPer50Frame(void)
{
	if (bossBeam.alive) bossBeamAct();//注意，boss生成beam的这一回合中，beam是不行动的，而beam行动的第一个回合就会杀死自己
	if (boss.alive) bossAct();
}
int main(void)
{
	gameStage = 1;
	system("mode con cols=101 lines=47  ");//设置控制台窗口大小
	hideCursor();
Head:
	selectStage();
	player.x = 20;
	player.y = 25;
	player.life = 5;
	player.alive = 1;
	boss.life = 5;//游戏靠检测boss.life==0来判断玩家胜利，所以boss一开始时尽管未出生，但是是满血
	boss.x = 20;
	boss.y = 0;
	bossBeam.alive = 0;
	bossBullet1.alive = 0;
	bossBullet2.alive = 0;
	score = 0;
	//文本界面初始化
	createANewAmmo();
	ammo[ammoNumber].location = 1;
	clearMessage();
	//游戏初始化
	roundsSeeingPlayer = 0;//boss持续看见玩家的回合数，见bossAct（）
	int numberOfBasicCycle = 0;//本次基本周期的编号
	gameStageTimmer = 1;
	flag = 1;
	successfulLoadingTimes = 0;
	gameStageTimmer = 0;
	pilotIsAlive = 0;
	playerCanControlFlight = 0;
	char thisBeamHasHurt = 0;

	//界面初始化
	srand(time(NULL));
	drawAll();

	//主循环，周期为50s，在对应的回合让对应的单位行动，然后检查碰撞，然后绘制界面
	while (1)
	{
		if (numberOfBasicCycle % 1 == 0)	runPerFrame();
		if (numberOfBasicCycle % 5 == 0)	runPer5Frame();
		if (numberOfBasicCycle % 10 == 0)	runPer10Frame();
		if (numberOfBasicCycle % 50 == 0)	runPer50Frame();

		checkContact();
		if(checkGameStage() == 2) goto Head;
		drawAll();
		
		numberOfBasicCycle++;

	}


}
/*
已解决：
玩家子弹和敌方子机碰撞却没有检测到，有时发生
	-子弹和敌机在相邻时相互交换位置，所以没有重合
画图太慢，导致不同的回合，绘制图像的时间不同。难以统一。
	-制作图像缓存区，准备好图像然后统一打印。使得绘图时间稳定在14ms

笔记：如何添加一个单位

声明它的character结构
编写它的绘制函数，并将其放入drawall（）
新建它的act函数，并将其放到对应的周期函数中
添加关于它的check内容
*/ 