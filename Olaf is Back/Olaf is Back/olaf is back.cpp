#include "PluginSDK.h"


PluginSetup("Olaf is Back ");

IMenu* MainMenu;

IMenu* ComboMenu;
IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboE;
IMenuOption* MinHP;

IMenu* HarassMenu;
IMenuOption* HarassQ;
IMenuOption* HarassQS;
IMenuOption* HarassE;
IMenuOption* HarassMana;

IMenu* KillstealMenu;
IMenuOption* KillstealE;
IMenuOption* KillstealQ;

IMenu* LaneMenu;
IMenuOption* LaneQ;
IMenuOption* LaneE;
IMenuOption* LaneMana;

IMenu* JungleMenu;
IMenuOption* JungClear;
IMenuOption* JungMana;
IMenuOption* JungKillE;

IMenu* DrawingMenu;
IMenuOption* DrawQRange;
IMenuOption* DrawQSRange;
IMenuOption* DrawERange;
IMenuOption* DrawAxe;

ISpell2* Q;
ISpell2* Q2;
ISpell2* W;
ISpell2* E;
IUnit* Player;
Vec3 AxePosition;
IUnit* Axe;

void Menu()
{
	MainMenu = GPluginSDK->AddMenu("Olaf is Back");
	ComboMenu = MainMenu->AddMenu("Combo Options");
	{
		
			ComboQ = ComboMenu->CheckBox("Use Q in Combo", true);
			ComboW = ComboMenu->CheckBox("Use W in Combo", true);
            ComboE = ComboMenu->CheckBox("Use E in Combo", true);
			MinHP  = ComboMenu->AddInteger("Minimum Hp % to E", 1, 100, 5);

	}

	HarassMenu = MainMenu->AddMenu("Harass Options");
	{
		HarassQ = HarassMenu->CheckBox("Use Q in harass", false);
		HarassQS = HarassMenu->CheckBox("Use Short Q in harass", true);
		HarassE = HarassMenu->CheckBox("Use E in harass", true);
		HarassMana = HarassMenu->AddInteger("Minimum Mana % to Harass", 1, 100, 60);
		
	}
	KillstealMenu = MainMenu->AddMenu(" KS Options");
	{
		KillstealQ = KillstealMenu->CheckBox("Use Q KS", true);
		KillstealE = KillstealMenu->CheckBox("Use E KS", true);

	}
	LaneMenu = MainMenu->AddMenu("Lane Clear Settings");
	{
		LaneQ = LaneMenu->CheckBox("Use Q in laneclear", true);
		LaneE = LaneMenu->CheckBox("Use E in laneclear", true);
		LaneMana= LaneMenu->AddInteger("Laneclear min mana %", 1, 100, 50);
	}

	JungleMenu = MainMenu->AddMenu("JungleClear Settings");
	{
		JungClear = JungleMenu->CheckBox("jungleclear", true);
		JungMana = JungleMenu->AddInteger("Jungleclear min mana %", 1, 100, 30);
		JungKillE= JungleMenu->CheckBox("jungle steal with E", true);
	}


	DrawingMenu = MainMenu->AddMenu("Drawing Settings");
	{
		DrawQRange = DrawingMenu->CheckBox("Draw Q Range", true);
		DrawQSRange = DrawingMenu->CheckBox("Draw Short Q Range", true);
		DrawERange = DrawingMenu->CheckBox("Draw E Range", true);
		DrawAxe = DrawingMenu->CheckBox("Draw Axe Position", true);
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
	Q->SetSkillshot(0.25f, 70, 1550, 1000);
	Q2 = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, true, kCollidesWithYasuoWall);
	Q2->SetSkillshot(0.25f, 70, 1550, 550);
	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, kCollidesWithNothing);
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, false, false, kCollidesWithNothing);
	E->SetOverrideRange(325);
}


void CastQ(IUnit* hit)
{
	auto time = 0.25f + GetDistance(Player, hit) / 1550;
	Vec3 futurePos;
	GPrediction->GetFutureUnitPosition(hit, time, true, futurePos);
	Vec2 hitpos = futurePos.To2D().Extend(Player->GetPosition().To2D(), -40);
	Vec3 Hithere;
	Hithere.x = hitpos.x;
	Hithere.y = futurePos.y;
	Hithere.z = hitpos.y;
	Q->CastOnPosition(Hithere);
}

void Combo()
{
	auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, 1000);
	if (target != nullptr)
	{
		if (ComboQ->Enabled() && Q->IsReady())
		{
			CastQ(target);
		}
		if (ComboE->Enabled() && E->IsReady() && Player->HealthPercent() >=MinHP->GetInteger() && GetDistance(Player, target) <= E->Range())
		{
			E->CastOnTarget(target);
		}
		if (ComboW->Enabled() && W->IsReady() && GetDistance(Player, target)<= 250)
		{
			W->CastOnPlayer();
		}
	}
}

void Harass()
{
	auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, 1000);
	if (target != nullptr)
	{
		if (HarassQ->Enabled() && Q->IsReady() && !HarassQS->Enabled() && Player->ManaPercent() > HarassMana->GetInteger())
		{
			CastQ(target);
		}
		if (HarassQS->Enabled() && Q->IsReady() && Player->ManaPercent() > HarassMana->GetInteger() && GetDistance(Player, target) <= Q2->Range())
		{
			Q2->CastOnTarget(target, kHitChanceHigh);
		}

		if (HarassE->Enabled() && E->IsReady() && Player->HealthPercent() >= MinHP->GetInteger() && GetDistance(Player, target) <= E->Range())
		{
			E->CastOnTarget(target);
		}


	}
}

void Killsteal()
{
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		if (Enemy != nullptr && !Enemy->IsDead())
		{
			if (KillstealQ->Enabled())
			{
				auto dmg = GDamage->GetSpellDamage(Player, Enemy, kSlotQ);
				if (Enemy->IsValidTarget(Player, Q->Range()) && !Enemy->IsInvulnerable())
				{
					if (Enemy->GetHealth() <= dmg + 20 && Q->IsReady())
					{
						CastQ(Enemy);
					}
				}
			}
			if (KillstealE->Enabled())
			{
				auto dmg = GDamage->GetSpellDamage(Player, Enemy, kSlotE);
				if (Enemy->IsValidTarget(Player, E->Range()) && !Enemy->IsInvulnerable())
				{
					if (Enemy->GetHealth() <= dmg + 20 && E->IsReady())
					{
						E->CastOnTarget(Enemy);
					}
				}
			}
		}
	}
}

void LaneClear()
{
	Vec3 position;
	int mincount;
	float lanehit=0;
	float lanehitex=0;
	float dist;

	if (LaneQ->Enabled() && Player->ManaPercent() > LaneMana->GetInteger() && Q->IsReady())
	{
		for (auto minion : GEntityList->GetAllMinions(false, true, false))
		{
			if (minion != nullptr && !minion->IsDead() && Player->IsValidTarget(minion, 1000))
			{
				dist = GetDistance(Player, minion);
				if (dist > lanehitex)
				{
					lanehitex = lanehit;
					lanehit = dist;
				}
			}
		}
		if (lanehit > 0)
		{
			GPrediction->FindBestCastPosition(lanehit, 130, true, true, false, position, mincount);

			if (mincount >= 2)
			{
				Q->CastOnPosition(position);
			}
		}

	}
	if (LaneE->Enabled() && E->IsReady())
	{
		for (auto minion : GEntityList->GetAllMinions(false, true, false))
		{

			if (minion != nullptr && !minion->IsDead() && Player->IsValidTarget(minion, E->Range()))
			{
				auto dmg = GDamage->GetSpellDamage(Player, minion, kSlotE);
				if (minion->GetHealth() <= dmg)
				{
					E->CastOnUnit(minion);
				}
			}
		}
		
	}
}
void Jungle()
{
	int mincount;
	Vec3 position;
	float junghit=0;
	float junghitex=0;
	float dist = 0;

	if (JungClear->Enabled()&& Player->ManaPercent() > JungMana->GetInteger())
	{
		for (auto minion : GEntityList->GetAllMinions(false, false, true))
		{
			if (minion != nullptr && !minion->IsDead() && Player->IsValidTarget(minion, 500))
			{
				if(Q->IsReady() && Player->IsValidTarget(minion, 500))
				{
					dist = GetDistance(Player, minion);
					if (dist > junghitex)
					{
						junghitex = junghit;
						junghit = dist;
						
					}
			    }
			if (junghit > 0)
			{
				GPrediction->FindBestCastPosition(junghit, 130, true, true, false, position, mincount);
				if (mincount > 1)
				{
					Q->CastOnPosition(position);
				}
				else
				{
					Q->CastOnUnit(minion);
				}
		    }
		
				if (W->IsReady() && Player->IsValidTarget(minion, 250))
				{
					W->CastOnPlayer();
				}

			}
		}
	}

}
void JungSteal()
{
	if (JungKillE->Enabled())
	{
		for (auto minion : GEntityList->GetAllMinions(false, false, true))
		{
			if (minion != nullptr && !minion->IsDead() && Player->IsValidTarget(minion, 325))
			{
				auto dmg = GDamage->GetSpellDamage(Player, minion, kSlotE);
				if (minion->GetHealth() <= dmg)
				{
					E->CastOnUnit(minion);
				}
			}

		}
	}
}

PLUGIN_EVENT(void) OnRender()
{
	if (DrawQRange->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range()); }
	if (DrawQSRange->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q2->Range()); }
	if (DrawERange->Enabled()) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range()); }
	if (DrawAxe->Enabled() && Axe != nullptr) { GPluginSDK->GetRenderer()->DrawOutlinedCircle(AxePosition, Vec4(255, 255, 0, 255), 100); }
}
PLUGIN_EVENT(void) OnCreateObject(IUnit* obj)
{
	if (strcmp(obj->GetObjectName(), "Olaf_Base_Q_Axe_Ally.troy") == 0)
	{
		Axe = obj;
		AxePosition = obj->GetPosition();
	}
}
PLUGIN_EVENT(void) OnDestroyObject(IUnit* obj)
{
	if (strcmp(obj->GetObjectName(), "Olaf_Base_Q_Axe_Ally.troy") == 0)
	{
		Axe = nullptr;
	}
}


PLUGIN_EVENT(void) OnGameUpdate()
{
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		Combo();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
	{
		LaneClear();
		Jungle();
	}
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed)
	{
		Harass();
	}
	Killsteal();
	JungSteal();
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

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();

	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnCreateObject, OnCreateObject);
	GEventManager->RemoveEventHandler(kEventOnDestroyObject, OnDestroyObject);
	
}
