// Copyright 2019 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "include.h"
#include "plateau.h"

#include <stdio.h>
#include <smk/Screen.hpp>


class Btn
{
	public:
		bool state[2];
    smk::Screen& screen;
		int k;
		Btn(smk::Screen& Screen,int k);
		void Update();
		bool State();
        int t_wait;
        bool mode;
};


Btn::Btn(smk::Screen& Screen,int K) :
	screen(Screen),
	k(K),
    t_wait(0),
    mode(false)
{
	state[0]=false;
	state[1]=false;
}

void Btn::Update()
{
  //bool press = screen.GetInput().IsKeyDown(k);
  bool press = false;
  state[1] = state[0];
  state[0] = press;
  if (mode == false) {
    if (press)
      t_wait++;
    else
      t_wait = 0;

    if (t_wait >= 10)
      mode = true;
  } else {
    if (not press)
      mode = false;
    state[0] = not state[1];
  }
}

bool Btn::State()
{
	return state[0] and not state[1];
}


int main()
{

	Plateau p;
	
	int choix;
	int width,height;
	width=10;
	height=10;
	string filename;
	int x,y;
	
	int x_cursor=0;
	int y_cursor=0;
	int blink=0;
	
	int mode=0;
	do
	{
		switch (mode)
		{
			case 0: // affichage du menu
				cout<<"[Menu]"<<endl;
				cout<<"       0) nouveau"<<endl;
				cout<<"       1) charger"<<endl;
				mode=1;
				break;
			case 1: // acquisition du choix
				cout<<"choix:"<<endl;
				cin>>choix;
				mode=2;
				break;
			
			case 2: // traitement du choix
				switch (choix)
				{
					case 0:	// nouveau
						cout<<"largeur du monde :";
						cin>> width;
						cout<<"hauteur du monde :";
						cin>> height;
						
						if ( width<=0 or height<=0 or width>=500 or height>=500)
						{
							cout<<"mauvaise entrée, recommencer"<<endl;
						}
						else
						{
							p.LoadEmpty(width,height);
							mode=-1;
						}
						
						break;
						
					case 1: // chargement
						cout<<"nom de fichier"<<endl;
						cin>>filename;
						p.Load((char*) filename.c_str());
						width=p.width;
						height=p.height;
						mode=-1;
						break;
					default:
						cout<<"choix invalide"<<endl;
						mode=1;
						break;
				}
				break;
			default:break;
		}
	} while(mode>=0);

	mode=0;
	
  smk::Screen screen(sf::VideoMode(width*16, height*16, 32), "Constructor");
	
	Shape rectangle=Shape::Rectangle(0,0,16,16,Color(255,255,255,255));
	
	Clock c;
	for ( int i=0;i<60;++i)
	{
		sf::Sleep(1.0/60.0-c.GetElapsedTime());
		c.Reset();
		screen.Clear(Color(0,0,0,255));
		screen.Draw(Shape::Rectangle(0,0,width*16.,height*16.,Color(255,255,255,255)));
		screen.Display();
	}
	Btn	keyLeft(screen,Key::Left);
	Btn	keyRight(screen,Key::Right);
	Btn	keyUp(screen,Key::Up);
	Btn	keyDown(screen,Key::Down);

	Btn keyO(screen,Key::O);
	Btn	keySpace(screen,Key::Space);
	Btn	keyEnter(screen,Key::Return);

	int outil=0;
	
	screen.Clear(Color(0,0,0,255));
	while(mode>=0)
	{
		sf::Sleep(1.0/60.0-c.GetElapsedTime());
		c.Reset();
		switch (mode)
		{
			case 0: // affichage
			{	
				for(x=0;x<p.width;x++)
				{
					for(y=0;y<p.height;y++)
					{
						rectangle.SetPosition(float(x)*16.,(float(p.height)-float(y)-1.)*16.);
						if (x==x_cursor and y==y_cursor and blink>=5)
						{
							if (blink>=10)
                                rectangle.SetColor(Color(255,255,255,40));
							else
                                rectangle.SetColor(Color(0,0,0,40));
                            screen.Draw(rectangle);	
						}
						else
						{
							switch (p.elements[x][y].type)
							{
								case vide:
									rectangle.SetColor(Color(40,40,40,40));
									break;
									
								case sol:
									rectangle.SetColor(Color(255,255,255,40));
									break;
									
								case bouton:
									rectangle.SetColor(Color(0,255,0,40));
									break;
								
								case retractable:
									rectangle.SetColor(Color(255,0,0,40));
									break;
								
								case finish:
									rectangle.SetColor(Color(0,0,255,40));
									break;

								case fragile:
									rectangle.SetColor(Color(255,0,255,40));
									break;

								default:break;
							}
							screen.Draw(rectangle);	
						}
					}
				}
				screen.Display();
				
				blink++;
				if (blink>15) blink=0;
				mode=1;
			} break;
			case 1: // acquisition action
			{	// flux event
				Event e;
				while(screen.GetEvent(e));
				keyLeft.Update();
				keyRight.Update();
				keyUp.Update();
				keyDown.Update();
				keySpace.Update();
				keyEnter.Update();
				keyO.Update();

				mode=2;
			} break;
			case 2: //traitement action
			{
				mode=0;
				if (keyDown.State())
						if(y_cursor>=1)
						y_cursor--;
				
				if (keyUp.State())
						if(y_cursor<=p.height-2)
						y_cursor++;

				if (keyLeft.State())
						if(x_cursor>=1)
						x_cursor--;

				if (keyRight.State())
						if(x_cursor<=p.width-2)
						x_cursor++; 
				
                if (keyDown.State() or keyUp.State() or keyLeft.State() or keyRight.State())
                    blink=10;

				#define NB_OUTILS 7;
				if (keyO.State())
				{
					outil=(outil+1)%NB_OUTILS;
					cout<<"outils:";
					switch(outil)
					{
						case 0: cout<<"vide"<<endl;break;
						case 1: cout<<"sol"<<endl;break;
						case 2: cout<<"bouton"<<endl;break;
						case 3: cout<<"finish"<<endl;break;
						case 4: cout<<"retractable"<<endl;break;
						case 5: cout<<"fragile"<<endl;break;
						case 6: cout<<"Position Initial"<<endl;break;
					}
				}

				if (keySpace.State())
				{
					switch(outil)
					{
						case 0:
						{
							p.elements[x_cursor][y_cursor].type=vide;
						} break;
						case 1:
						{
							p.elements[x_cursor][y_cursor].type=sol;
						} break;
						case 2:
						{
							int nbTarget;
							int x,y;
							int weight;
							string type;
							p.elements[x_cursor][y_cursor].type=bouton;
							cout<<"poid";
							cin>>weight;
							cout<<"nombre de boutons à activer:";
							cin>>nbTarget;
							p.elements[x_cursor][y_cursor].bouton.nbTarget=nbTarget;
							p.elements[x_cursor][y_cursor].bouton.minWeight=weight;
							p.elements[x_cursor][y_cursor].bouton.target=new BoutonSignal[nbTarget];
							for (int i = 0; i < nbTarget; i++) {
								cout<<"x:";
								cin>>x;
								cout<<"y:";
								cin>>y;
								cout<<"type (on,off,invert):";
								cin>>type;
								p.elements[x_cursor][y_cursor].bouton.target[i].x=x;
								p.elements[x_cursor][y_cursor].bouton.target[i].y=y;
								if (type=="off")
									p.elements[x_cursor][y_cursor].bouton.target[i].mode=BoutonMode::Off;
								else if (type=="on")
									p.elements[x_cursor][y_cursor].bouton.target[i].mode=BoutonMode::On;
								else if (type=="invert")
									p.elements[x_cursor][y_cursor].bouton.target[i].mode=BoutonMode::Invert;

							}
						} break;
						case 3:
						{
							p.elements[x_cursor][y_cursor].type=finish;
						}	break;
						case 4:
						{
							p.elements[x_cursor][y_cursor].type=retractable;
							cout<<"direction d'appuie: (left,right,up,down):";
							string input;
							cin>>input;
									if (input=="left") p.elements[x_cursor][y_cursor].retractable.d=direction_type::left;
							else	if (input=="up") p.elements[x_cursor][y_cursor].retractable.d=direction_type::up;
							else	if (input=="right") p.elements[x_cursor][y_cursor].retractable.d=direction_type::right;
							else	if (input=="down") p.elements[x_cursor][y_cursor].retractable.d=direction_type::down;

							cout<<"etat de base:(bas,haut):";
							cin>>input;
									if (input=="bas") p.elements[x_cursor][y_cursor].retractable.state=false;
							else	if (input=="haut") p.elements[x_cursor][y_cursor].retractable.state=true;
						} break;
						case 5:
						{
							p.elements[x_cursor][y_cursor].type=fragile;
							//cout<<"poid maximum:";
							//int poid;
							//cin>>poid;
							//p.elements[x_cursor][y_cursor].fragile.maxWeight=poid;
							p.elements[x_cursor][y_cursor].fragile.maxWeight=1;
						}	break;
						case 6:
						{
							cout<<"position block"<<endl<<"x:";
							cin>>p.block.x;
							cout<<"y:";
							cin>>p.block.y;
							cout<<"a:";
							cin>>p.block.a;
							cout<<"b:";
							cin>>p.block.b;
							cout<<"c:";
							cin>>p.block.c;
						} break;
					}
				}
				if (keyEnter.State())
					mode=3;

				
			} break;
			case 3:
				cout<<"nom de fichier pour enregistrer"<<endl;
				cin>>filename;
				p.Save((char*)filename.c_str());
				mode=-1;
				break;
			
		}		
	}
}
