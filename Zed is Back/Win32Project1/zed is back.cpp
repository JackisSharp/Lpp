#include "PluginSDK.h"
#include <string>


PluginSetup("Zed is Back ");

IMenu* MainMenu;

IMenu* ComboMenu;
IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboE;
IMenuOption* ComboR;
IMenuOption* CheckDMG;


IMenu* HarassMenu;
IMenuOption* HarassQ;
IMenuOption* HarassLong;
IMenuOption* HarassE;
IMenuOption* HarassW;

IMenu* ItemMenu;
IMenuOption* ItemB;
IMenuOption* ItemC;
IMenuOption* ItemY;
IMenuOption* SumI;


IMenu* DrawingMenu;
IMenuOption* DrawQRange;
IMenuOption* DrawQSRange;
IMenuOption* DrawERange;
IMenuOption* DrawShPos;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;
ISpell2* Ignite;
IUnit* Player;
IUnit* RShadow;
IUnit* WShadow;
Vec3 rpos;
Vec3 wpos;
IInventoryItem* cutlass;
IInventoryItem* botrk;
IInventoryItem* youmuus;

int shadowdelay;
int delayw=600;
int LastCastedSpell;


void Menu()
{
	MainMenu = GPluginSDK->AddMenu("Zed is Back");
	ComboMenu = MainMenu->AddMenu("Combo Options");
	{

		ComboQ = ComboMenu->CheckBox("Use Q in Combo", true);
		ComboW = ComboMenu->CheckBox("Use W in Combo also gap close", true);
		ComboE = ComboMenu->CheckBox("Use E in Combo", true);
		ComboR = ComboMenu->CheckBox("Use R in Combo", true);
		CheckDMG = ComboMenu->CheckBox("Dont use R if target is killable with Q+E", true);

	}

	HarassMenu = MainMenu->AddMenu("Harass Options");
	{
		HarassQ = HarassMenu->CheckBox("Use Q in harass", true);
		HarassLong = HarassMenu->CheckBox("Use Long distance harass", false);
		HarassE = HarassMenu->CheckBox("Use E in harass", true);
		HarassW = HarassMenu->CheckBox("Use W in harass", true);
	}

	ItemMenu = MainMenu->AddMenu("Item and Summoner Options");
	{
		ItemB = ItemMenu->CheckBox("Use Botrk in combo", true);
		ItemC = ItemMenu->CheckBox("Use Cutlass in combo", true);
		ItemY = ItemMenu->CheckBox("Use Youmuu's in combo", false);
		SumI = ItemMenu->CheckBox("Use ignite in combo", true);
	
	}



	DrawingMenu = MainMenu->AddMenu("Drawing Settings");
	{
		DrawQRange = DrawingMenu->CheckBox("Draw Q Range", true);
		DrawQSRange = DrawingMenu->CheckBox("Draw R Range", true);
		DrawERange = DrawingMenu->CheckBox("Draw E Range", true);
		DrawShPos = DrawingMenu->CheckBox("Draw Shadow Positions", true);
	}
}

float GetDistance(IUnit* source, IUnit* target)
{
	auto x1 = source->GetPosition().x;
	auto x2 = target->GetPosition().x;
	auto y1 = source->GetPosition().y;
	auto y2 = target->GetPosition().y;
	auto z1 = source->GetPosition().z;
	auto z2 = target->GetPosition().z;
	return static_cast<float>(sqrt(pow((x2 - x1), 2.0) + pow((y2 - y1), 2.0) + pow((z2 - z1), 2.0)));
}

void LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, true, kCollidesWithYasuoWall);
	Q->SetSkillshot(0.25f, 50, 1700, 900);
	W = GPluginSDK->CreateSpell2(kSlotW, kLineCast, false, false, kCollidesWithNothing);
	W->SetOverrideRange(680);
	E = GPluginSDK->CreateSpell2(kSlotE, kCircleCast, false, false, kCollidesWithNothing);
	E->SetOverrideRange(270);
	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, false, false, kCollidesWithNothing);
	R->SetOverrideRange(625);

	if (strcmp(GEntityList->Player()->GetSpellName(kSummonerSlot1), "SummonerDot") == 0)
	{
		Ignite = GPluginSDK->CreateSpell2(kSummonerSlot1, kTargetCast, false, false, kCollidesWithNothing);
		Ignite->SetOverrideRange(600);
	}
	if (strcmp(GEntityList->Player()->GetSpellName(kSummonerSlot2), "SummonerDot") == 0)
	{
		Ignite = GPluginSDK->CreateSpell2(kSummonerSlot2, kTargetCast, false, false, kCollidesWithNothing);
		Ignite->SetOverrideRange(600);
	}

	cutlass = GPluginSDK->CreateItemForId(3144, 550);
	botrk = GPluginSDK->CreateItemForId(3153, 550);
	youmuus = GPluginSDK->CreateItemForId(3142, 0);
}





void CastQ(IUnit* hit)
{
	auto time = 0.25f + GetDistance(Player, hit) / 1600;
	Vec3 futurePos;
	GPrediction->GetFutureUnitPosition(hit, time, true, futurePos);
	Vec2 hitpos = futurePos.To2D().Extend(Player->GetPosition().To2D(), -20);
	Vec3 Hithere;
	Hithere.x = hitpos.x;
	Hithere.y = futurePos.y;
	Hithere.z = hitpos.y;
	Q->CastOnPosition(Hithere);

}
void CastW(IUnit* hit)
{
	if (delayw >= GGame->TickCount() - shadowdelay)
	{
		return;
	}
		
	Vec3 herew = hit->ServerPosition().Extend(Player->ServerPosition(), -200);
	W->CastOnPosition(herew);
	shadowdelay = GGame->TickCount();	
}

void CastE()
{
	if (!E->IsReady())
	{
		return;
	}
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		if (Enemy->IsValidTarget() && !Enemy->IsDead()&& (GetDistance(Player, Enemy)< E->Range()|
		   ( WShadow!=nullptr&&GetDistance(WShadow, Enemy)<E->Range())| (RShadow != nullptr&&GetDistance(RShadow, Enemy)<E->Range())))
		{
			E->CastOnPlayer();
		}
	}

}

void Combo()
{
	auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, 1300);
	
	if (target != nullptr && target->IsValidTarget() && !target->IsDead())
	{
		auto overkill = GDamage->GetSpellDamage(Player, target, kSlotQ) + GDamage->GetSpellDamage(Player, target, kSlotE);
		if (ComboR->Enabled()&&R->IsReady()&& RShadow==nullptr&&(!CheckDMG->Enabled()|target->GetHealth() > overkill))
		{
			if (ComboW->Enabled() && ((GetDistance(Player, target) > 625 && target->MovementSpeed() > Player->MovementSpeed())| GetDistance(Player, target) > 800))
			{
				Vec3 herew = target->ServerPosition().Extend(Player->ServerPosition(), -200);
				W->CastOnPosition(herew);
			}
			if (GetDistance(Player, target) <= 625)
			{
				R->CastOnTarget(target);
				if (Ignite != nullptr&& Ignite->IsReady())
				{
					Ignite->CastOnTarget(target);
				}
			}

		}
		else 
		{
			if (Ignite!=nullptr&& Ignite->IsReady()&& GetDistance(Player,target)<600 && (target->GetHealth()<
				(GDamage->GetSpellDamage(Player, target, kSlotQ)*2 + GDamage->GetSpellDamage(Player, target, kSlotE)*2)))
			{
				Ignite->CastOnTarget(target);
			}
			if(ComboW->Enabled()&& GetDistance(Player, target) > 400 && GetDistance(Player, target) < 1200 && LastCastedSpell!=4)
			{
				Vec3 herew = target->ServerPosition().Extend(Player->ServerPosition(), -200);
				W->CastOnPosition(herew);
			}

			CastE();
			if (ItemC->Enabled() && cutlass->IsOwned() && cutlass->IsReady()&& GetDistance(Player, target)<550)
			{
				cutlass->CastOnTarget(target);
			}
			if (ItemB->Enabled() && botrk->IsOwned() && botrk->IsReady()&& GetDistance(Player, target)<550)
			{
				botrk->CastOnTarget(target);
			}
			if (ItemY->Enabled() && youmuus->IsOwned() && youmuus->IsReady() && GetDistance(Player, target)<900)
			{
				youmuus->CastOnPlayer();
			}

			if (ComboQ->Enabled() && Q->IsReady() &&  ((RShadow != nullptr&&GetDistance(RShadow, target)<850)|GetDistance(Player, target)<850 | 
				(WShadow != nullptr&&GetDistance(WShadow, target)<850)))
			{
				CastQ(target);

			}

		}
		
	}
}


void Harass()
{
	auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, 1400);
	if (target != nullptr)
	{
		if (HarassLong->Enabled()&& !target->IsDead() && target->IsValidTarget() && W->IsReady()&&  WShadow == nullptr&& Q->IsReady() &&
			Player->GetMana()>(Q->ManaCost() + W->ManaCost())&& GetDistance(Player, target)>900)
		{
			CastW(target);
		}
		if (HarassQ->Enabled() && Q->IsReady()&& !target->IsDead() && target->IsValidTarget()&&(!W->IsReady()|!HarassW->Enabled()|WShadow!=nullptr)
			&& (GetDistance(Player, target)<850 |(WShadow != nullptr&&GetDistance(WShadow, target)<850)))
		{
			CastQ(target);

		}
		if (HarassW->Enabled()&&!target->IsDead()&& target->IsValidTarget()&&W->IsReady() && Q->IsReady()&& WShadow==nullptr&& GetDistance(Player,target)<850
			&& Player->GetMana()>(Q->ManaCost()+ W->ManaCost()+E->ManaCost()))
		{
			CastW(target);
		}

		CastE();


	}
}
void Farm()
{

}
PLUGIN_EVENT(void) OnRender()
{
	if (DrawQRange->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range()); }
	if (DrawQSRange->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range()); }
	if (DrawERange->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }
	if (DrawShPos->Enabled()&& RShadow!=nullptr) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(rpos, Vec4(255, 255, 0, 255), 100); }
	if (DrawShPos->Enabled()&& WShadow!=nullptr) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(wpos, Vec4(80, 184, 84, 255), 100); }
	
}

	
PLUGIN_EVENT(void) OnCreateObject(IUnit* obj)	
{
	if (RShadow!=nullptr && strcmp(obj->GetObjectName(), "Zed_Base_CloneDeath.troy") == 0 && GetDistance(obj, RShadow)<10)
	{
		RShadow = nullptr;
	}
	if (WShadow != nullptr && strcmp(obj->GetObjectName(), "Zed_Base_CloneDeath.troy") == 0 && GetDistance(obj, WShadow)<10)
	{
		WShadow = nullptr;
	}
		
}
	
PLUGIN_EVENT(void) OnSpellCast(CastedSpell const& Args)
{
	if (Args.Caster_ == Player)
	{
		if (strcmp(Args.Name_, "ZedQ") == 0)
		{
			LastCastedSpell = 1;
		}
		if ((strcmp(Args.Name_, "ZedW") == 0)| (strcmp(Args.Name_, "ZedW2") == 0))
		{
			LastCastedSpell = 2;
		}
		if (strcmp(Args.Name_, "ZedE") == 0)
		{
			LastCastedSpell = 3;
		}
		if ((strcmp(Args.Name_, "ZedR") == 0 )| (strcmp(Args.Name_, "ZedR2") == 0))
		{
			LastCastedSpell = 4;		
		}
		

	}
}


PLUGIN_EVENT(void) OnDestroyObject(IUnit* obj)
{

}




PLUGIN_EVENT(void) OnGameUpdate()
{
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		Combo();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
	{
		Farm();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed)
	{
		Harass();
	}
	if (LastCastedSpell == 2)
	{
		
		for (auto unit : GEntityList->GetAllMinions(true, false, false))
		{
			if (unit != nullptr && strcmp(unit->GetObjectName(), "Shadow") == 0 && !unit->IsDead()&&unit->IsVisible() && unit != RShadow)
			{
				wpos = unit->ServerPosition();
				WShadow = unit;
				
				return;

			}

		}
		
	}
	if (LastCastedSpell == 4)
	{

		for (auto unit : GEntityList->GetAllMinions(true, false, false))
		{
			if (unit != nullptr && strcmp(unit->GetObjectName(), "Shadow") == 0 && !unit->IsDead()&&unit->IsVisible()&&unit!=WShadow)
			{
				rpos = unit->ServerPosition();
				RShadow = unit;
				return;

			}

		}
		
	}

}



PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	LoadSpells();
	Player = GEntityList->Player();

	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
	GEventManager->AddEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->AddEventHandler(kEventOnDestroyObject, OnDestroyObject);
	GEventManager->AddEventHandler(kEventOnSpellCast, OnSpellCast);

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();

	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->RemoveEventHandler(kEventOnDestroyObject, OnDestroyObject);
	GEventManager->RemoveEventHandler(kEventOnSpellCast, OnSpellCast);

}
