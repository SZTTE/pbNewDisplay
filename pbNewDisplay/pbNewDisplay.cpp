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
//check if is commited
//待用变量,应该再一开始先声明有什么变量，再主程序里再给他们赋值
int score;
int gameStage;//游戏阶段：1子机对战，2Boss战
char imageCache[52][102];//图像缓存，用来储存将要输出的内容 [y][x]
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
	if (GetKeyState(VK_UP) < 0) return 1;
	if (GetKeyState(VK_DOWN) < 0) return 2;
	if (GetKeyState(VK_LEFT) < 0) return 3;
	if (GetKeyState(VK_RIGHT) < 0) return 4;
	return 0;
}
//空格键状态:1按下，0松开
int stateOfSpace(void)
{
	if (GetKeyState(VK_SPACE) < 0) return 1;
	else return 0;
}
//清空图像缓存区
void clearImageCache(void)
{
	for (int y = 0; y <= 50; y++) 
	{
		for (int x = 0; x <= 100; x++)
		{
			imageCache[y][x] = 'a';
		}
		imageCache[y][101] = '\0';
	}
}
//将缓存区内的图像打印到屏幕上
void displayImageCache(void)
{
	setCursorAt(0, 0);
	for (int y = 0; y <= 50; y++)
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

//===================================================================二层 基础工具=========================================================================================================================================================================================================


int line = 2;//当前行数
//调试用，把信息显示在info栏
void putInformation(const char message[50])
{
	setCursorAt(44, line);
	printf(message);
	line++;
}

//===================================================================三层 画画=========================================================================================================================================================================================================

//绘制血量界面
void drawPlayerLifeInterface(void)
{
	setCursorAt(9, 42);
	printf("PLAYER LIFE:");
	for (int i = 1; i <= 5; i++)
	{
		if (player.life >= i)
		{
			printf("0");
		}
		else {
			printf("<");
		}
	}
}
void drawBossLifeInterface(void)
{
	setCursorAt(9, 43);
	printf("  BOSS LIFE:");
	for (int i = 1; i <= 10; i++)
	{
		if (boss.life >= i)
		{
			printf("0");
		}
		else {
			printf("<");
		}
	}
}
//绘制基本界面：游戏边框
void drawBasicInterface(void)
{
	//绘制第一排的白板，boss的部分身体有时候出现在那里，需要遮盖
	setCursorAt(1, 0);
	printf("                                        ");
	//绘制边框
	setCursorAt(1, 1);
	printf(" -------------------------------------- ");
	for (int i = 2; i != 40; i++)
	{
		setCursorAt(1, i);
		printf("|");
		setCursorAt(40, i);
		printf("|");
	}
	setCursorAt(1, 40);
	printf(" -------------------------------------- ");
}
//绘制静态界面A：下面的标题，静止画等，A表示这是在玩家得到移动授权之前的界面
void drawStaticInterfaceA(void)
{
	setCursorAt(1, 32);
	printf("通信对象：\n\n");
	printf(" 延迟：\n\n");
	printf(" 中继：\n\n");
	printf(" 航向校正");
}
//绘制人物下面的白板
void drawBackground()
{
	
	for (int a = 0; a <= 39; a++)
	{
		setCursorAt(2, a);
		printf("                                      ");
	}
}
//绘制分数界面
void drawScoreInterface(void)
{
	setCursorAt(15, 41);
	printf("SCORE:%d", score);
}
//绘制玩家飞机
void drawPlayer(void)
{
	setCursorAt(player.x - 4, player.y - 3);
	printf("        \n");
	setCursorAt(player.x - 4, player.y - 2);
	printf("        \n");
	setCursorAt(player.x - 4, player.y - 1);
	printf("        \n");
	setCursorAt(player.x - 4, player.y - 0);
	printf("        \n");
	setCursorAt(player.x - 4, player.y +1);
	printf("          \n");
	setCursorAt(player.x - 4, player.y +2);
	printf("          \n");
	setCursorAt(player.x, player.y - 2);
	printf("A");
	setCursorAt(player.x, player.y - 1);
	printf("H");
	setCursorAt(player.x - 1, player.y);
	printf("/P\\");
	setCursorAt(player.x - 3, player.y + 1);
	printf("-=qop=-");
}
//绘制玩家子弹
void drawPlayerBullet(void)
{
	setCursorAt(playerBullet.x, playerBullet.y);
	printf("|");
}
//绘制敌方子机
void drawEnermyChild(void)
{
	setCursorAt(enermyChild.x, enermyChild.y);
	printf("T");
}
//绘制BOSS飞机
void drawBoss(void)
{
	setCursorAt(boss.x, boss.y + 2);
	printf("V");
	setCursorAt(boss.x, boss.y + 1);
	printf("H");
	setCursorAt(boss.x - 1, boss.y);
	printf("\\B/");
	setCursorAt(boss.x - 3, boss.y - 1);
	printf("-=dob=-");
}
//绘制BOSS子弹
void drawBossBullet1(void)
{
	setCursorAt(bossBullet1.x, bossBullet1.y);
	printf("|");
}
void drawBossBullet2(void)
{
	setCursorAt(bossBullet2.x, bossBullet2.y);
	printf("|");
}
//绘制BOSS激光
void drawBossBeam(void)
{
	for (int i = bossBeam.y + 3; i <= 39; i++)
	{
		setCursorAt(bossBeam.x, i);
		printf("!");
	}
}
//===================================================================四层 高级工具、结算界面=========================================================================================================================================================================================================
void drawAll(void)
{
	drawBackground();
	drawScoreInterface();
	drawPlayerLifeInterface();
	if (enermyChild.alive) drawEnermyChild();
	if (bossBullet1.alive) drawBossBullet1();
	if (bossBullet2.alive) drawBossBullet2();
	if (playerBullet.alive) drawPlayerBullet();
	if (bossBeam.alive) drawBossBeam();
	drawPlayer();
	if (boss.alive) drawBossLifeInterface();
	if (boss.alive) drawBoss();
	drawBasicInterface();
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
	printf("GAME OVER\n");
	Sleep(1000);
	exit(0);
}
void gameClear(void)
{
	system("cls");
	printf("GAME CLEAR\n");
	printf("Your score is %d\n", score);
	Sleep(1000);
	exit(0);
}

//===================================================================顶层 行动函数+碰撞检测==========================================================================================================================================================================================================
void playerAct()
{
	//1.玩家移动
	switch (stateOfArrowKey())
	{
		case 1:
			player.y--;
			break;
		case 2:
			player.y++;
			break;
		case 3:
			player.x--;
			break;
		case 4:
			player.x++;
			break;
	}
	//2.检查移动后是否触碰边界，要是会碰到就不要动了
	if (player.x - 3 == 1) player.x++;
	if (player.x + 3 == 40) player.x--;
	if (player.y - 2 == 1) player.y++;
	if (player.y +1 == 40) player.y--;
	//3.发射子弹
	if (playerBullet.alive == 0 && stateOfSpace())
	{
		playerBullet.alive = 1;
		playerBullet.x = player.x;
		playerBullet.y = player.y - 3;
	}
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
	if (enermyChild.y == 40) enermyChild.alive = 0;
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
		bossBullet1.alive = 1;
		bossBullet1.x = boss.x - 2;
		bossBullet1.y = boss.y + 2;
		bossBullet2.alive = 1;
		bossBullet2.x = boss.x + 2;
		bossBullet2.y = boss.y + 2;
		bossBeam.alive = 1;
		bossBeam.x = boss.x;
		bossBeam.y = boss.y;

		roundsSeeingPlayer = 0;
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
	if (bossBullet1.y == 40) bossBullet1.alive = 0;
	drawBossBullet1();
}
void bossBullet2Act()
{
	//移动
	bossBullet2.y++;
	if (bossBullet2.y == 40) bossBullet2.alive = 0;
	drawBossBullet2();
}
void bossBeamAct()
{
	bossBeam.alive = 0;
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

	//要是玩家和boss激光纵向重合，玩家扣血
	if (player.x == bossBeam.x)
	{
		player.life--;
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
void checkGameStage()
{
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
	if (player.life == 0) gameOver();
}
//===================================================================计时器结构，将行动函数填入对应周期的大括号中======================================================================================================================================
void runPer50ms(void)
{

	if (playerBullet.alive) playerBulletAct();
	if (bossBullet1.alive) bossBullet1Act();
	if (bossBullet2.alive) bossBullet2Act();
}
void runPer100ms(void)
{
	playerAct();
	if (enermyChild.alive == 1) enermyChildAct();
}
void runPer500ms(void)
{
	if (bossBeam.alive) bossBeamAct();//注意，boss生成beam的这一回合中，beam是不行动的，而beam行动的第一个回合就会杀死自己
	if (boss.alive) bossAct();
}
int main(void)
{
	//
	int timeStart = time(NULL);
	clearImageCache();
	writeImageCache(3, 1, "test");
	displayImageCache();
	getchar();
	//变量初始化
	player.x = 20;
	player.y = 30;
	player.life = 5;
	player.alive = 1;
	boss.life = 10;//游戏靠检测boss.life==0来判断玩家胜利，所以boss一开始时尽管未出生，但是是满血
	score = 0;
	gameStage = 1;
	roundsSeeingPlayer = 0;//boss持续看见玩家的回合数，见bossAct（）
	int numberOfBasicCycle = 0;//本次基本周期的编号

	//界面初始化
	//system("mode con cols=42 lines=45  ");//设置控制台窗口大小
	hideCursor();
	srand(time(NULL));
	drawAll();

	//主循环，周期为50s，在对应的回合让对应的单位行动，然后检查碰撞，然后绘制界面
	while (1)
	{
		if (numberOfBasicCycle % 1 == 0)	runPer50ms();
		if (numberOfBasicCycle % 2 == 0)	runPer100ms();
		if (numberOfBasicCycle % 10 == 0)	runPer500ms();

		checkContact();
		checkGameStage();
		drawBasicInterface();
		//drawBackground();
		//drawAll();
		//
		//Sleep(100);

		if (numberOfBasicCycle == 1000)
		{
			putInformation("在100回合后，时间增加了：");
			printf("%d",time(NULL) - timeStart);
			getchar();

		}
		numberOfBasicCycle++;

	}


}
/*
已解决：
玩家子弹和敌方子机碰撞却没有检测到，有时发生
	-子弹和敌机在相邻时相互交换位置，所以没有重合
写到boss'act

笔记：如何添加一个单位

声明它的character结构
编写它的绘制函数，并将其放入drawall（）
新建它的act函数，并将其放到对应的周期函数中
添加关于它的check内容
*/ 
