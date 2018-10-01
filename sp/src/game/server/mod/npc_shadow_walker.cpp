//========= Copyright Valve Corporation, All rights reserved. ============//
// This is a skeleton file for use when creating a new 
// NPC. Copy and rename this file for the new
// NPC and add the copy to the build.
//
// Leave this file in the build until we ship! Allowing 
// this file to be rebuilt with the rest of the game ensures
// that it stays up to date with the rest of the NPC code.
//
// Replace occurances of CNPC_ShadowWalker with the new NPC's
// classname. Don't forget the lower-case occurance in 
// LINK_ENTITY_TO_CLASS()
//
//
// ASSUMPTIONS MADE:
//
// You're making a character based on CAI_BaseNPC. If this 
// is not true, make sure you replace all occurances
// of 'CAI_BaseNPC' in this file with the appropriate 
// parent class.
//
// You're making a human-sized NPC that walks.
//
//=============================================================================//
#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "activitylist.h"
#include "ai_basenpc.h"
#include "engine/IEngineSound.h"
#include "basehlcombatweapon_shared.h"
#include "ai_squadslot.h"
//#include "npc_BaseZombie.h"
//#include "vehicle_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
// Private activities
//=========================================================
//int	ACT_MYCUSTOMACTIVITY = -1;

//=========================================================
// Custom schedules
//=========================================================
//enum
//{
//	SCHED_MYCUSTOMSCHEDULE = LAST_SHARED_SCHEDULE,
//};

//=========================================================
// Custom tasks
//=========================================================
//enum 
//{
//	TASK_MYCUSTOMTASK = LAST_SHARED_TASK,
//};


//=========================================================
// Custom Conditions
//=========================================================
//enum 
//{
//	COND_MYCUSTOMCONDITION = LAST_SHARED_CONDITION,
//};

//=========================================================
// Convars
//=========================================================

ConVar	sk_shadow_walker_dmg_innate_melee("sk_zombie_dmg_one_slash", "0");

//=========================================================
//=========================================================
class CNPC_ShadowWalker : public CAI_BaseNPC
{
	DECLARE_CLASS( CNPC_ShadowWalker, CAI_BaseNPC );

public:
	void	Precache( void );
	void	Spawn( void );
	Class_T Classify( void );
	virtual int				SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode);
	virtual int 			SelectScheduleRetrieveItem();
	virtual int 			SelectSchedule();
	virtual int				SelectIdleSchedule();
	virtual int				SelectAlertSchedule();
	virtual int				SelectCombatSchedule();
	virtual bool			CanPickkUpWeapons() { return true;  }

	// Citizen methods
	Activity		NPC_TranslateActivity(Activity eNewActivity);

	// Zombie methods
	//void HandleAnimEvent(animevent_t *pEvent);
	//virtual CBaseEntity *ClawAttack(float flDist, int iDamage, QAngle &qaViewPunch, Vector &vecVelocityPunch, int BloodOrigin);
	//virtual float GetClawAttackRange() const { return ZOMBIE_MELEE_REACH; }
	
	DECLARE_DATADESC();

	// This is a dummy field. In order to provide save/restore
	// code in this file, we must have at least one field
	// for the code to operate on. Delete this field when
	// you are ready to do your own save/restore for this
	// character.
	bool		m_bHasWeapon;

	DEFINE_CUSTOM_AI;

private:
	bool		HasRangedWeapon();
};


LINK_ENTITY_TO_CLASS( npc_shadow_walker, CNPC_ShadowWalker );
IMPLEMENT_CUSTOM_AI( npc_citizen,CNPC_ShadowWalker );


//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC( CNPC_ShadowWalker )

	DEFINE_FIELD(m_bHasWeapon, FIELD_BOOLEAN),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Initialize the custom schedules
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::InitCustomSchedules(void) 
{
	INIT_CUSTOM_AI(CNPC_ShadowWalker);

	//ADD_CUSTOM_TASK(CNPC_ShadowWalker,		TASK_MYCUSTOMTASK);

	//ADD_CUSTOM_SCHEDULE(CNPC_ShadowWalker,	SCHED_MYCUSTOMSCHEDULE);

	//ADD_CUSTOM_ACTIVITY(CNPC_ShadowWalker,	ACT_MYCUSTOMACTIVITY);

	//ADD_CUSTOM_CONDITION(CNPC_ShadowWalker,	COND_MYCUSTOMCONDITION);
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::Precache( void )
{
	if (!GetModelName())
	{
		SetModelName(MAKE_STRING("models/monster/subject.mdl"));
	}

	PrecacheModel(STRING(GetModelName()));
	BaseClass::Precache();
}


//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CNPC_ShadowWalker::Spawn( void )
{
	Precache();

	SetModel(STRING(GetModelName()));
	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
	m_iHealth			= 50; // Replace this with setting from Hammer
	m_flFieldOfView		= 0.5;
	m_NPCState			= NPC_STATE_NONE;

	CapabilitiesClear();

	if (!HasSpawnFlags(SF_NPC_START_EFFICIENT))
	{
		// CapabilitiesAdd(bits_CAP_ANIMATEDFACE);
		CapabilitiesAdd(bits_CAP_TURN_HEAD);
		CapabilitiesAdd(bits_CAP_SQUAD);
		CapabilitiesAdd(bits_CAP_USE_WEAPONS | bits_CAP_AIM_GUN | bits_CAP_MOVE_SHOOT);
		CapabilitiesAdd(bits_CAP_WEAPON_MELEE_ATTACK1 || bits_CAP_WEAPON_MELEE_ATTACK2);
		CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1 || bits_CAP_INNATE_MELEE_ATTACK2);
		CapabilitiesAdd(bits_CAP_DUCK | bits_CAP_DOORS_GROUP);
		CapabilitiesAdd(bits_CAP_USE_SHOT_REGULATOR);
	}

	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	SetMoveType(MOVETYPE_STEP);



	NPCInit();
}


//-----------------------------------------------------------------------------
// Purpose: Choose a schedule after schedule failed
// Copied from npc_citizen
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode)
{
	switch (failedSchedule)
	{
	case SCHED_NEW_WEAPON:
		// If failed trying to pick up a weapon, try again in one second. This is because other AI code
		// has put this off for 10 seconds under the assumption that the citizen would be able to 
		// pick up the weapon that they found. 
		m_flNextWeaponSearchTime = gpGlobals->curtime + 1.0f;
		break;
	case SCHED_TAKE_COVER_FROM_ENEMY:
		// I can't take cover, so I need to run away!
		return SCHED_RUN_FROM_ENEMY;
	case SCHED_CHASE_ENEMY:
		// I can't run away, so I will just run randomly!
		return SCHED_CHASE_ENEMY_FAILED;
	case SCHED_RUN_FROM_ENEMY:
		// I can't run away, so I will just run randomly!
		return SCHED_RUN_RANDOM;
	}

	return BaseClass::SelectFailSchedule(failedSchedule, failedTask, taskFailCode);
}

//-----------------------------------------------------------------------------
// Copied from npc_citizen
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectScheduleRetrieveItem()
{
	if (HasCondition(COND_BETTER_WEAPON_AVAILABLE))
	{
		CBaseHLCombatWeapon *pWeapon = dynamic_cast<CBaseHLCombatWeapon *>(Weapon_FindUsable(WEAPON_SEARCH_DELTA));
		if (pWeapon)
		{
			m_flNextWeaponSearchTime = gpGlobals->curtime + 10.0;
			// Now lock the weapon for several seconds while we go to pick it up.
			pWeapon->Lock(10.0, this);
			SetTarget(pWeapon);
			return SCHED_NEW_WEAPON;
		}
	}

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
// Copied from npc_citizen - used as SelectSchedule instead of SelectPriorityAction
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectSchedule()
{
	switch (m_NPCState)
	{
	case NPC_STATE_IDLE:
		AssertMsgOnce(GetEnemy() == NULL, "NPC has enemy but is not in combat state?");
		return SelectIdleSchedule();

	case NPC_STATE_ALERT:
		AssertMsgOnce(GetEnemy() == NULL, "NPC has enemy but is not in combat state?");
		return SelectAlertSchedule();

	case NPC_STATE_COMBAT:
		return SelectCombatSchedule();

	default:
		return BaseClass::SelectSchedule();
	}
}

//-----------------------------------------------------------------------------
// Idle schedule selection
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectIdleSchedule()
{
	int nSched = SelectFlinchSchedule();
	if (nSched != SCHED_NONE)
		return nSched;

	if (HasCondition(COND_HEAR_DANGER) ||
		HasCondition(COND_HEAR_COMBAT) ||
		HasCondition(COND_HEAR_WORLD) ||
		HasCondition(COND_HEAR_BULLET_IMPACT) ||
		HasCondition(COND_HEAR_PLAYER))
	{
		// Investigate sound source
		return SCHED_INVESTIGATE_SOUND;
	}

	if (CanPickkUpWeapons() && HasCondition(COND_BETTER_WEAPON_AVAILABLE)) {
		nSched = SelectScheduleRetrieveItem();
		if (nSched != SCHED_NONE)
			return nSched;
	}

	// no valid route! Wander instead
	if (GetNavigator()->GetGoalType() == GOALTYPE_NONE)
		return SCHED_IDLE_WANDER;

	// valid route. Get moving
	return SCHED_IDLE_WALK;
}

//-----------------------------------------------------------------------------
// Alert schedule selection
// Copied from baseNPC
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectAlertSchedule()
{
	// Per default base NPC, check flinch schedule first
	int nSched = SelectFlinchSchedule();
	if (nSched != SCHED_NONE)
		return nSched;

	// Scan around for new enemies
	if (HasCondition(COND_ENEMY_DEAD) && SelectWeightedSequence(ACT_VICTORY_DANCE) != ACTIVITY_NOT_AVAILABLE)
		return SCHED_ALERT_SCAN;

	if (HasCondition(COND_HEAR_DANGER) ||
		HasCondition(COND_HEAR_PLAYER) ||
		HasCondition(COND_HEAR_WORLD) ||
		HasCondition(COND_HEAR_BULLET_IMPACT) ||
		HasCondition(COND_HEAR_COMBAT))
	{
		// Investigate sound source
		return SCHED_INVESTIGATE_SOUND;
	}

	if (CanPickkUpWeapons() && HasCondition(COND_BETTER_WEAPON_AVAILABLE)) {
		nSched = SelectScheduleRetrieveItem();
		if (nSched != SCHED_NONE)
			return nSched;
	}

	// no valid route! Wander instead
	if (GetNavigator()->GetGoalType() == GOALTYPE_NONE)
		return SCHED_IDLE_WANDER;

	// valid route. Get moving
	return SCHED_ALERT_WALK;
}

//-----------------------------------------------------------------------------
// Combat schedule selection
//-----------------------------------------------------------------------------
int CNPC_ShadowWalker::SelectCombatSchedule()
{
	// Check flinch first
	int nSched = SelectFlinchSchedule();
	if (nSched != SCHED_NONE)
		return nSched;

	// Check enemy death
	if (HasCondition(COND_ENEMY_DEAD))
	{
		// clear the current (dead) enemy and try to find another.
		SetEnemy(NULL);

		if (ChooseEnemy())
		{
			ClearCondition(COND_ENEMY_DEAD);
			return SelectSchedule();
		}

		SetState(NPC_STATE_ALERT);
		return SelectSchedule();
	}

	// If I'm scared of this enemy and he's looking at me, run away
	// If in a squad, all but one Shadow walker must be afraid of the player
	if (IRelationType(GetEnemy()) == D_FR || !OccupyStrategySlotRange(SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2))
	{
		if (HasCondition(COND_SEE_ENEMY) && HasCondition(COND_ENEMY_FACING_ME)  && HasCondition(COND_HAVE_ENEMY_LOS))
		{
			// TODO: Check if silent
			FearSound();
			return SCHED_RUN_FROM_ENEMY;
		}
	}

	// Reloading conditions are necessary just in case for some reason somebody gives the Shadow Walker a gun
	if (HasCondition(COND_LOW_PRIMARY_AMMO) || HasCondition(COND_NO_PRIMARY_AMMO))
	{
		return SCHED_HIDE_AND_RELOAD;
	}

	// Can we see the enemy?
	if (!HasCondition(COND_SEE_ENEMY))
	{
		// chase!
		return SCHED_CHASE_ENEMY;
	}

	if (HasCondition(COND_TOO_CLOSE_TO_ATTACK))
		return SCHED_BACK_AWAY_FROM_ENEMY;


	// we can see the enemy
	if (HasCondition(COND_CAN_MELEE_ATTACK1)) {
			return SCHED_MELEE_ATTACK1;
	}

	if (HasCondition(COND_CAN_MELEE_ATTACK2)) {
			return SCHED_MELEE_ATTACK2;
	}

	if (HasRangedWeapon() && GetShotRegulator()->IsInRestInterval())
	{
		if (HasCondition(COND_CAN_RANGE_ATTACK1))
			return SCHED_COMBAT_FACE;
	}

	if (HasRangedWeapon() && HasCondition(COND_CAN_RANGE_ATTACK1))
	{
		if (OccupyStrategySlotRange(SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2))
			return SCHED_RANGE_ATTACK1;
		return SCHED_RUN_FROM_ENEMY;
	}

	if (HasRangedWeapon() && HasCondition(COND_CAN_RANGE_ATTACK2))
		return SCHED_RANGE_ATTACK2;


	if (HasCondition(COND_NOT_FACING_ATTACK))
		return SCHED_COMBAT_FACE;

	if (!HasCondition(COND_CAN_RANGE_ATTACK1) && !HasCondition(COND_CAN_MELEE_ATTACK1))
	{
		// if we can see enemy but can't use either attack type, we must need to get closer to enemy
		if (HasRangedWeapon())
			return SCHED_MOVE_TO_WEAPON_RANGE;

		return SCHED_CHASE_ENEMY;
	}

	DevWarning(2, "No suitable combat schedule!\n");
	return SCHED_FAIL;
}

bool CNPC_ShadowWalker::HasRangedWeapon()
{
	CBaseCombatWeapon *pWeapon = GetActiveWeapon();

	if (pWeapon)
		return !(FClassnameIs(pWeapon, "weapon_crowbar") || FClassnameIs(pWeapon, "weapon_stunstick"));

	return false;
}

//-----------------------------------------------------------------------------
// Originally copied from Citizen
// Purpose: Override base class activiites
//-----------------------------------------------------------------------------
Activity CNPC_ShadowWalker::NPC_TranslateActivity(Activity activity)
{
	if (activity == ACT_MELEE_ATTACK1)
	{
		return ACT_MELEE_ATTACK_SWING;
	}

	// !!!HACK - Citizens don't have the required animations for shotguns, 
	// so trick them into using the rifle counterparts for now (sjb)
	if (activity == ACT_RUN_AIM_SHOTGUN)
		return ACT_RUN_AIM_RIFLE;
	if (activity == ACT_WALK_AIM_SHOTGUN)
		return ACT_WALK_AIM_RIFLE;
	if (activity == ACT_IDLE_ANGRY_SHOTGUN)
		return ACT_IDLE_ANGRY_SMG1;
	if (activity == ACT_RANGE_ATTACK_SHOTGUN_LOW)
		return ACT_RANGE_ATTACK_SMG1_LOW;

	return BaseClass::NPC_TranslateActivity(activity);
}

//-----------------------------------------------------------------------------
// Originally copied from BaseZombie
// Purpose: Catches the monster-specific events that occur when tagged animation
//			frames are played.
// Input  : pEvent - 
//-----------------------------------------------------------------------------
//void CNPC_ShadowWalker::HandleAnimEvent(animevent_t *pEvent)
//{
//	if (pEvent->event == AE_ZOMBIE_ATTACK_RIGHT)
//	{
//		Vector right, forward;
//		AngleVectors(GetLocalAngles(), &forward, &right, NULL);
//
//		right = right * 100;
//		forward = forward * 200;
//
//		QAngle qa(-15, -20, -10);
//		Vector vec = right + forward;
//		ClawAttack(GetClawAttackRange(), sk_shadow_walker_dmg_innate_melee.GetFloat(), qa, vec, ZOMBIE_BLOOD_RIGHT_HAND);
//		return;
//	}
//
//	if (pEvent->event == AE_ZOMBIE_ATTACK_LEFT)
//	{
//		Vector right, forward;
//		AngleVectors(GetLocalAngles(), &forward, &right, NULL);
//
//		right = right * -100;
//		forward = forward * 200;
//
//		QAngle qa(-15, 20, -10);
//		Vector vec = right + forward;
//		ClawAttack(GetClawAttackRange(), sk_shadow_walker_dmg_innate_melee.GetFloat(), qa, vec, ZOMBIE_BLOOD_LEFT_HAND);
//		return;
//	}
//
//	if (pEvent->event == AE_ZOMBIE_ATTACK_BOTH)
//	{
//		Vector forward;
//		QAngle qaPunch(45, random->RandomInt(-5, 5), random->RandomInt(-5, 5));
//		AngleVectors(GetLocalAngles(), &forward);
//		forward = forward * 200;
//		ClawAttack(GetClawAttackRange(), sk_shadow_walker_dmg_innate_melee.GetFloat(), qaPunch, forward, ZOMBIE_BLOOD_BOTH_HANDS);
//		return;
//	}
//
//	BaseClass::HandleAnimEvent(pEvent);
//}

//-----------------------------------------------------------------------------
// Originally copied from basezombie
// Purpose: Look in front and see if the claw hit anything.
//
// Input  :	flDist				distance to trace		
//			iDamage				damage to do if attack hits
//			vecViewPunch		camera punch (if attack hits player)
//			vecVelocityPunch	velocity punch (if attack hits player)
//
// Output : The entity hit by claws. NULL if nothing.
//-----------------------------------------------------------------------------
//CBaseEntity *CNPC_ShadowWalker::ClawAttack(float flDist, int iDamage, QAngle &qaViewPunch, Vector &vecVelocityPunch, int BloodOrigin)
//{
//	DevWarning(1, "Claw attack!\n");
//
//	// Added test because claw attack anim sometimes used when for cases other than melee
//	int iDriverInitialHealth = -1;
//	CBaseEntity *pDriver = NULL;
//	if (GetEnemy())
//	{
//		trace_t	tr;
//		AI_TraceHull(WorldSpaceCenter(), GetEnemy()->WorldSpaceCenter(), -Vector(8, 8, 8), Vector(8, 8, 8), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
//
//		if (tr.fraction < 1.0f)
//			return NULL;
//
//		// CheckTraceHullAttack() can damage player in vehicle as side effect of melee attack damaging physics objects, which the car forwards to the player
//		// need to detect this to get correct damage effects
//		CBaseCombatCharacter *pCCEnemy = (GetEnemy() != NULL) ? GetEnemy()->MyCombatCharacterPointer() : NULL;
//		CBaseEntity *pVehicleEntity;
//		if (pCCEnemy != NULL && (pVehicleEntity = pCCEnemy->GetVehicleEntity()) != NULL)
//		{
//			if (pVehicleEntity->GetServerVehicle() && dynamic_cast<CPropVehicleDriveable *>(pVehicleEntity))
//			{
//				pDriver = static_cast<CPropVehicleDriveable *>(pVehicleEntity)->GetDriver();
//				if (pDriver && pDriver->IsPlayer())
//				{
//					iDriverInitialHealth = pDriver->GetHealth();
//				}
//				else
//				{
//					pDriver = NULL;
//				}
//			}
//		}
//	}
//
//	//
//	// Trace out a cubic section of our hull and see what we hit.
//	//
//	Vector vecMins = GetHullMins();
//	Vector vecMaxs = GetHullMaxs();
//	vecMins.z = vecMins.x;
//	vecMaxs.z = vecMaxs.x;
//
//	CBaseEntity *pHurt = NULL;
//	if (GetEnemy() && GetEnemy()->Classify() == CLASS_BULLSEYE)
//	{
//		// We always hit bullseyes we're targeting
//		pHurt = GetEnemy();
//		CTakeDamageInfo info(this, this, vec3_origin, GetAbsOrigin(), iDamage, DMG_SLASH);
//		pHurt->TakeDamage(info);
//	}
//	else
//	{
//		// Try to hit them with a trace
//		pHurt = CheckTraceHullAttack(flDist, vecMins, vecMaxs, iDamage, DMG_SLASH);
//	}
//
//	if (pDriver && iDriverInitialHealth != pDriver->GetHealth())
//	{
//		pHurt = pDriver;
//	}
//
//	if (pHurt)
//	{
//		//AttackHitSound();
//
//		CBasePlayer *pPlayer = ToBasePlayer(pHurt);
//
//		if (pPlayer != NULL && !(pPlayer->GetFlags() & FL_GODMODE))
//		{
//			pPlayer->ViewPunch(qaViewPunch);
//
//			pPlayer->VelocityPunch(vecVelocityPunch);
//		}
//		else if (!pPlayer && UTIL_ShouldShowBlood(pHurt->BloodColor()))
//		{
//			// Hit an NPC. Bleed them!
//			Vector vecBloodPos;
//
//			switch (BloodOrigin)
//			{
//			case ZOMBIE_BLOOD_LEFT_HAND:
//				if (GetAttachment("blood_left", vecBloodPos))
//					SpawnBlood(vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), MIN(iDamage, 30));
//				break;
//
//			case ZOMBIE_BLOOD_RIGHT_HAND:
//				if (GetAttachment("blood_right", vecBloodPos))
//					SpawnBlood(vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), MIN(iDamage, 30));
//				break;
//
//			case ZOMBIE_BLOOD_BOTH_HANDS:
//				if (GetAttachment("blood_left", vecBloodPos))
//					SpawnBlood(vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), MIN(iDamage, 30));
//
//				if (GetAttachment("blood_right", vecBloodPos))
//					SpawnBlood(vecBloodPos, g_vecAttackDir, pHurt->BloodColor(), MIN(iDamage, 30));
//				break;
//
//			case ZOMBIE_BLOOD_BITE:
//				// No blood for these.
//				break;
//			}
//		}
//	}
//	else
//	{
//		//AttackMissSound();
//	}
//
//
//	return pHurt;
//}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_ShadowWalker::Classify( void )
{
	return	CLASS_ZOMBIE;
}
