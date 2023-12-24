#include"Headers/VS/include/easyx.h"
#include<bits/stdc++.h>
#include<sysinfoapi.h>
#include<winuser.h>
#define Window_Width 1280      
#define Window_Height 720
#define Button_Width 192
#define Button_Height 75
#pragma comment(lib,"MSIMG32.LIB") 
#pragma comment(lib,"Winmm.lib")   

typedef unsigned long long int scf;      //Player's Score
bool is_game_started = false;
bool running = true;
bool is_choose_bk = false;
bool is_choose_FPS = false;
unsigned int background_num = 0;
unsigned int FPS = 144;
IMAGE img_background;

inline void putimage_alpha(int x, int y, IMAGE* img)
{
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h, GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}

class Button
{
public:
	Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
	{
		region = rect;

		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);
	}

	~Button() = default;

	void EventProcess(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
				status = Status::Hovered;
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
				status = Status::Idle;
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
				status = Status::Pushed;
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed)
				OnClick();
			break;
		default:
			break;
		}
	}

	void Draw()
	{
		switch (status)
		{
		case Button::Status::Idle:
			putimage(region.left, region.top, &img_idle);
			break;
		case Button::Status::Hovered:
			putimage(region.left, region.top, &img_hovered);
			break;
		case Button::Status::Pushed:
			putimage(region.left, region.top, &img_pushed);
			break;
		default:
			break;
		}
	}
protected:
	virtual void OnClick() = 0;

private:
	enum class Status
	{
		Idle=1,
		Hovered,
		Pushed
	};
private:
	RECT region;
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;
private:
	bool CheckCursorHit(int x, int y) const
	{
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}
};

class StartGameButton : public Button
{
public:
	StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~StartGameButton() = default;
protected:
	void OnClick()
	{
		is_game_started = true;
	}
};

class QuitGameButton : public Button
{
public:
	QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~QuitGameButton() = default;
protected:
	void OnClick()
	{
		running = false;
	}
};

class ChooseBK1Button : public Button
{
public:
	ChooseBK1Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect,path_img_idle,path_img_hovered,path_img_pushed) {}
	~ChooseBK1Button() = default;
protected:
	void OnClick()
	{
		loadimage(&img_background, _T("Images/bk/background1.png"));
		
		is_choose_bk = true;
	}
};

class ChooseBK2Button : public Button
{
public:
	ChooseBK2Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~ChooseBK2Button() = default;
protected:
	void OnClick()
	{
		loadimage(&img_background, _T("Images/bk/background2.png"));
		
		is_choose_bk = true;
	}
};

class Choose60FPSButton : public Button
{
public:
	Choose60FPSButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~Choose60FPSButton() = default;
protected:
	void OnClick()
	{
		FPS = 60;
		is_choose_FPS = true;
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
	}
};

class Choose144FPSButton : public Button
{
public:
	Choose144FPSButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~Choose144FPSButton() = default;
protected:
	void OnClick()
	{
		FPS = 144;
		is_choose_FPS = true;
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
	}
};


class Atlas
{
public:
	Atlas(LPCTSTR path, int num)
	{
		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)
		{
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);
		}
	}
	~Atlas()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
			delete frame_list[i];
	}

public:
	std::vector<IMAGE*> frame_list;
};

Atlas* atlas_player_left;
Atlas* atlas_player_right;
Atlas* atlas_enemy_left;
Atlas* atlas_enemy_right;

class Animation
{
public:
	Animation(Atlas* atlas, int interval)
	{
		anim_atlas = atlas;
		interval_ms = interval;
	}
	
	~Animation() = default;

	inline void play(int x, int y, int delta)
	{
		timer += delta;

		if (timer >= interval_ms)
		{
			idx_frame = (idx_frame + 1) % anim_atlas->frame_list.size();
			timer = 0;
		}

		putimage_alpha(x, y, anim_atlas->frame_list[idx_frame]);
	}
	
private:
	int timer = 0;
	int idx_frame = 0;
	int interval_ms = 0;
private:
	Atlas* anim_atlas;
};

class Player
{
public:
	const int FRAME_WIDTH = 80;
	const int FRAME_HEIGHT = 80;

public:
	inline Player()
	{
		loadimage(&shadow_player, _T("Images/Player/Player1/shadow_player.png"));            
		anim_left = new Animation(atlas_player_left, 45);    
		anim_right = new Animation(atlas_player_right, 45);  
	}

	inline ~Player()
	{
		delete anim_left;
		delete anim_right;
	}

	inline void ProcessEvent(const ExMessage& msg)
	{
		if (msg.message == WM_KEYDOWN)   
		{
			switch (msg.vkcode)           
			{
			case VK_UP:             
				is_move_up = true;
				break;

			case VK_DOWN:             
				is_move_down = true;
				break;

			case VK_LEFT:                 
				is_move_left = true;
				break;

			case VK_RIGHT:                 
				is_move_right = true;
				break;

			}
		}
		else if (msg.message == WM_KEYUP)  
		{                                  
			switch (msg.vkcode)           
			{
			case VK_UP:                  
				is_move_up = false;
				break;

			case VK_DOWN:                
				is_move_down = false;
				break;

			case VK_LEFT:                  
				is_move_left = false;
				break;

			case VK_RIGHT:                 
				is_move_right = false;
				break;
			}
		}
	}

	inline void Move()
	{
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);

		if (len_dir != 0)//如果行走
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			 
			Player_Pos.x += (int)(PLAYER_SPEED * normalized_x);
			Player_Pos.y += (int)(PLAYER_SPEED * normalized_y);
		}

		if (Player_Pos.x < 0) Player_Pos.x = 0;                                                            
		if (Player_Pos.y < 0) Player_Pos.y = 0;                                                            
		if (Player_Pos.x + PLAYER_WIDTH > Window_Width) Player_Pos.x = Window_Width - PLAYER_WIDTH;        
		if (Player_Pos.y + PLAYER_HEIGHT > Window_Height)Player_Pos.y = Window_Height - PLAYER_HEIGHT;     
		
	}

	inline void Draw(int delta)
	{
		int pos_shadow_x = Player_Pos.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = Player_Pos.y + PLAYER_HEIGHT - 8;
		static bool facing_left = false;
		int dir_x = is_move_right - is_move_left;

		putimage_alpha(pos_shadow_x, pos_shadow_y, &shadow_player);

		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;

		if (facing_left)
			anim_left->play(Player_Pos.x, Player_Pos.y, delta);
		else
			anim_right->play(Player_Pos.x, Player_Pos.y, delta);
	}

	inline const POINT& GetPosition() const
	{
		return Player_Pos;
	}

private:
	const int PLAYER_SPEED = 3;
	const int PLAYER_WIDTH = 80;
	const int PLAYER_HEIGHT = 80;
	const int SHADOW_WIDTH = 32;
	const int Player_Amin_Num = 6;
	int index_currect_anim = 0;
	Animation* anim_left;
	Animation* anim_right;
	IMAGE shadow_player;
	POINT Player_Pos = { 500 ,500 };
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;

};

class Bullet
{
public:
	POINT position = { 0,0 };
public:
	Bullet() = default;
	~Bullet() = default;

	void Draw() const
	{
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(position.x, position.y, RADIUS);
	}
private:
	const int RADIUS = 10;
};

class Enemy
{
public:
	Enemy()
	{
		loadimage(&img_shadow, _T("Images/Player/Enemy/shadow_enemy.png"));
		anim_left = new Animation(atlas_enemy_left, 45);
		anim_right = new Animation(atlas_enemy_right, 45);

		enum class SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right
		};

		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge)
		{
		case SpawnEdge::Up:
			position.x = rand() % Window_Width;
			position.y = -FRAME_HIGHT;
			break;

		case SpawnEdge::Down:
			position.x = rand() % Window_Width;
			position.y = Window_Height;
			break;

		case SpawnEdge::Left:
			position.x = -FRAME_WIDTH;
			position.y = rand() % Window_Height;
			break;

		case SpawnEdge::Right:
			position.x = Window_Width;
			position.y = rand() % Window_Height;
			break;

		default:
			break;
		}
	}

	inline bool CheckBulletCollision(const Bullet& bullet) const
	{
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HIGHT;

		return is_overlap_x && is_overlap_y;
	}

	inline bool CheckPlayerCollision(const Player& player)
	{
		POINT check_position = { position.x + FRAME_WIDTH / 2,position.y + FRAME_HIGHT / 2 };

		bool is_collision_x = check_position.x >= player.GetPosition().x && check_position.x <= player.GetPosition().x + player.FRAME_WIDTH;
		bool is_collision_y = check_position.y >= player.GetPosition().y && check_position.y <= player.GetPosition().y + player.FRAME_HEIGHT;

		return is_collision_x && is_collision_y;
	}
	inline void Move(const Player& player)
	{
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);

		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(SPEED * normalized_x);
			position.y += (int)(SPEED * normalized_y);
		}
	}

	~Enemy()
	{
		delete anim_left;
		delete anim_right;
	}

	inline void Draw(int delta)
	{
		int pos_shadow_x = position.x + (FRAME_WIDTH / 2 - SHADOW_WIDHT / 2);
		int pos_shadow_y = position.y + FRAME_HIGHT - 35;
		bool facing_left = false;
		int dir_x = is_move_right - is_move_left;

		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		if (dir_x > 0)
			facing_left = true;
		else if (dir_x < 0)
			facing_left = false;

		if (facing_left)
			anim_left->play(position.x, position.y, delta);
		else
			anim_right->play(position.x, position.y, delta);
	}

	inline const POINT& GetPosition() const
	{
		return position;
	}

	inline bool CheckAlive() const
	{
		return alive;
	}

	inline void Hurt()
	{
		alive = false;
	}

private:
	const int SPEED = 2;
	const int FRAME_HIGHT = 80;
	const int FRAME_WIDTH = 80;
	const int SHADOW_WIDHT = 48;
	
private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };
	bool facing_left = false;
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;
	bool alive = true;
	
};

void TryCreateEnemy(std::vector<Enemy*>& enemy_list)
{
	const int INTERVAL = 100;
	static int counter = 0;

	if ((++counter) % INTERVAL == 0)
	{
		enemy_list.push_back(new Enemy());
	}
}

void UploadBullets(std::vector<Bullet>& bullet_list, const Player& player)
{
	const double RADIAL_SPEED = 0.0045;
	const double TANGENT_SPEED = 0.0055;
	double radian_interval = 2 * 3.14159 / bullet_list.size();
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);

	for (size_t i = 0; i < bullet_list.size(); i++)
	{
		double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;
		bullet_list[i].position.x = player_position.x + player.FRAME_WIDTH / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.FRAME_HEIGHT / 2 + (int)(radius * cos(radian));
	}
}

void DrawPlayerScore(scf score)
{
	static TCHAR text[64];
	_stprintf_s(text, _T("当前分数:%d"), (int)score);

	setbkmode(TRANSPARENT);
	settextcolor(RGB(255, 85, 185));
	outtextxy(10, 10, text);
}

int main()
{
	atlas_enemy_left = new Atlas(_T("Images/Player/Enemy/enemy_left_%d.png"), 6);
	atlas_enemy_right = new Atlas(_T("Images/Player/Enemy/enemy_right_%d.png"), 6);
	atlas_player_left = new Atlas(_T("Images/Player/Player1/player_left_%d.png"), 6);
	atlas_player_right = new Atlas(_T("Images/Player/Player1/player_right_%d.png"), 6);
	scf score = 0;
	Player player;
	ExMessage msg;
	IMAGE img_menu;
	std::vector<Enemy*> enemy_list;
	std::vector<Bullet> bullet_list(3);
	RECT region_btn_start_game = {}, region_btn_quit_game = {};

	region_btn_start_game.left = (Window_Width - Button_Width) / 2;
	region_btn_start_game.right = region_btn_start_game.left + Button_Width;
	region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + Button_Height;

	region_btn_quit_game.left = (Window_Width - Button_Width) / 2;
	region_btn_quit_game.right = region_btn_quit_game.left + Button_Width;
	region_btn_quit_game.top = 550;
	region_btn_quit_game.bottom = region_btn_quit_game.top + Button_Height;

	StartGameButton btn_start_game = StartGameButton(
		region_btn_start_game,
		_T("Images/button/ui_start_idle.png"),
		_T("Images/button/ui_start_hovered.png"),
		_T("Images/button/ui_start_pushed.png"));

	QuitGameButton btn_quit_game = QuitGameButton(
		region_btn_quit_game,
		_T("Images/button/ui_quit_idle.png"),
		_T("Images/button/ui_quit_hovered.png"), 
		_T("Images/button/ui_quit_pushed.png"));

	ChooseBK1Button btn_chs_bk1 = ChooseBK1Button(
		region_btn_start_game,
		_T("Images/button/ui_bk1_idle.png"), 
		_T("Images/button/ui_bk1_hovered.png"), 
		_T("Images/button/ui_bk1_pushed.png"));

	ChooseBK2Button btn_chs_bk2 = ChooseBK2Button(
		region_btn_quit_game,
		_T("Images/button/ui_bk2_idle.png"), 
		_T("Images/button/ui_bk2_hovered.png"), 
		_T("Images/button/ui_bk2.pushed.png"));

	Choose60FPSButton btn_chs_F60 = Choose60FPSButton(
		region_btn_start_game,
		_T("Images/button/ui_60_idle.png"),
		_T("Images/button/ui_60_hovered.png"),
		_T("Images/button/ui_60_pushed.png"));

	Choose144FPSButton btn_chs_F144 = Choose144FPSButton(
		region_btn_quit_game,
		_T("Images/button/ui_144_idle.png"), 
		_T("Images/button/ui_144_hovered.png"), 
		_T("Images/button/ui_144_pushed.png"));

	initgraph(1280, 720);

	loadimage(&img_menu, _T("Images/bk/menu.png"));

	mciSendString(_T("open Music/bgm.mp3 alias bgm"), NULL, 0, NULL);
	mciSendString(_T("open Music/hit.wav alias hit"), NULL, 0, NULL);

	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;

	BeginBatchDraw();

	while (running)
	{
		DWORD start_time = GetTickCount();

		while (peekmessage(&msg))
		{
			if(is_game_started)
			{
				if (is_choose_bk == true && is_choose_FPS == true)
				{
					player.ProcessEvent(msg);
				}
				else if (is_choose_bk == false)
				{
					btn_chs_bk1.EventProcess(msg);
					btn_chs_bk2.EventProcess(msg);
				}
				else if (is_choose_bk == true && is_choose_FPS == false)
				{
					btn_chs_F60.EventProcess(msg);
					btn_chs_F144.EventProcess(msg);
				}
			}
			else
			{
				btn_start_game.EventProcess(msg);
				btn_quit_game.EventProcess(msg);
			}
		}
		
		if(is_game_started && is_choose_bk && is_choose_FPS)
		{
			player.Move();
			TryCreateEnemy(enemy_list);
			UploadBullets(bullet_list, player);

			for (Enemy* enemy : enemy_list)
				enemy->Move(player);

			for (Enemy* enemy : enemy_list)
			{
				if (enemy->CheckPlayerCollision(player))
				{
					static TCHAR text[64];

					_stprintf_s(text, _T("最终分数:%d!"), (int)score);

					MessageBox(GetHWnd(), text, _T("Game Over"), (MB_OK, MB_HELP));
					running = false;
					break;
				}
			}

			for (Enemy* enemy : enemy_list)
			{
				for (const Bullet& bullet : bullet_list)
				{
					if (enemy->CheckBulletCollision(bullet))
					{
						mciSendString(_T("play hit from 0"), NULL, 0, NULL);

						enemy->Hurt();
						score++;
					}
				}
			}

			for (size_t i = 0; i < enemy_list.size(); i++)
			{
				Enemy* enemy = enemy_list[i];

				if (!enemy->CheckAlive())
				{
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();
					delete enemy;
				}
			}
		}

		cleardevice();

		/*绘制模块*/
		if(is_game_started && is_choose_bk && is_choose_FPS)
		{
			putimage(0, 0, &img_background);
			player.Draw(1000 / FPS);                  
			for (Enemy* enemy : enemy_list) 
				enemy->Draw(1000 / FPS);                 
			for (const Bullet& bullet : bullet_list)     
				bullet.Draw(); 
			DrawPlayerScore(score);
		}
		else if(is_game_started == false)
		{
			putimage(0, 0, &img_menu);
			btn_start_game.Draw();
			btn_quit_game.Draw();
		}
		else if (is_game_started == true && is_choose_bk == false)
		{
			putimage(0, 0, &img_menu);
			btn_chs_bk1.Draw();
			btn_chs_bk2.Draw();
		}
		else if (is_game_started == true && is_choose_bk == true && is_choose_FPS == false)
		{
			putimage(0, 0, &img_menu);
			btn_chs_F60.Draw();
			btn_chs_F144.Draw();
		}


		FlushBatchDraw();

		DWORD end_time = GetTickCount();
		//结束时间
		DWORD delta_time = end_time - start_time;
		//单次while循环执行时间

		if (delta_time < 1000 / FPS)
		{
			Sleep(1000 / FPS - delta_time);
		}
		//补帧模块
	}

	delete atlas_enemy_left;
	delete atlas_enemy_right;
	delete atlas_player_left;
	delete atlas_player_right;

	EndBatchDraw();

	return (int)score;
}