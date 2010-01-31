#include <iostream>

#include "helpers.h"
#include "evetime.h"
#include "character.h"

Character::Character (EveApiAuth const& auth)
{
  this->auth = auth;

  this->cs_fetcher.set_auth(auth);
  this->ts_fetcher.set_auth(auth);
  this->sq_fetcher.set_auth(auth);

  this->cs_fetcher.set_doctype(API_DOCTYPE_CHARSHEET);
  this->ts_fetcher.set_doctype(API_DOCTYPE_INTRAINING);
  this->sq_fetcher.set_doctype(API_DOCTYPE_SKILLQUEUE);

  this->cs = ApiCharSheet::create();
  this->ts = ApiInTraining::create();
  this->sq = ApiSkillQueue::create();

  this->cs_fetcher.signal_done().connect(sigc::mem_fun
      (*this, &Character::on_cs_available));
  this->ts_fetcher.signal_done().connect(sigc::mem_fun
      (*this, &Character::on_ts_available));
  this->sq_fetcher.signal_done().connect(sigc::mem_fun
      (*this, &Character::on_sq_available));

  this->process_api_data();
}

/* ---------------------------------------------------------------- */

Character::~Character (void)
{
  //std::cout << "Removing character" << std::endl;
}

/* ---------------------------------------------------------------- */

void
Character::on_cs_available (EveApiData data)
{
  if (data.data.get() == 0)
  {
    this->sig_request_error.emit(API_DOCTYPE_CHARSHEET, data.exception);
    return;
  }

  bool yet_unnamed = this->cs->name.empty();

  try
  {
    this->cs->set_api_data(data);
    if (data.locally_cached)
      this->sig_cached_warning.emit(API_DOCTYPE_CHARSHEET, data.exception);
  }
  catch (Exception& e)
  {
    this->sig_request_error.emit(API_DOCTYPE_CHARSHEET, e);
    return;
  }

  if (yet_unnamed && !this->cs->name.empty())
    this->sig_name_available.emit(this->auth.char_id);

  this->process_api_data();
  this->sig_char_sheet_updated.emit();
  this->sig_api_info_changed.emit();
}

/* ---------------------------------------------------------------- */

void
Character::on_ts_available (EveApiData data)
{
  if (data.data.get() == 0)
  {
    this->sig_request_error.emit(API_DOCTYPE_INTRAINING, data.exception);
    return;
  }

  try
  {
    this->ts->set_api_data(data);
    if (data.locally_cached)
      this->sig_cached_warning.emit(API_DOCTYPE_INTRAINING, data.exception);
  }
  catch (Exception& e)
  {
    this->sig_request_error.emit(API_DOCTYPE_INTRAINING, e);
    return;
  }

  this->process_api_data();
  this->sig_intraining_updated.emit();
  this->sig_api_info_changed.emit();
}

/* ---------------------------------------------------------------- */

void
Character::on_sq_available (EveApiData data)
{
  if (data.data.get() == 0)
  {
    this->sig_request_error.emit(API_DOCTYPE_SKILLQUEUE, data.exception);
    return;
  }

  try
  {
    this->sq->set_api_data(data);
    if (data.locally_cached)
      this->sig_cached_warning.emit(API_DOCTYPE_SKILLQUEUE, data.exception);
  }
  catch (Exception& e)
  {
    this->sig_request_error.emit(API_DOCTYPE_SKILLQUEUE, e);
    return;
  }

  //this->process_api_data();
  this->sig_skill_queue_updated.emit();
  this->sig_api_info_changed.emit();
}

/* ---------------------------------------------------------------- */

void
Character::process_api_data (void)
{
  /* Update information related to the character sheet. */
  if (this->cs->valid)
  {
    this->char_base_sp = this->cs->total_sp;
    this->char_live_sp = this->cs->total_sp;
  }
  else
  {
    this->char_base_sp = 0;
    this->char_live_sp = 0;
  }

  /* Update information related to the training sheet. */
  if (this->ts->valid && this->ts->in_training)
  {
    ApiSkillTreePtr tree = ApiSkillTree::request();
    this->training_skill = tree->get_skill_for_id(this->ts->skill);
    this->training_spph = this->ts->get_current_spph();

    if (this->training_skill == 0)
    {
      std::cout << "Warning: Skill in training (ID " << this->ts->skill
          << ") was not found. Skill tree out of date?" << std::endl;
    }

    if (!this->cs->valid)
      this->update_live_info();
  }
  else
  {
    this->training_skill = 0;
    this->training_spph = 0;
  }

  /* Update information related to both sheets. */
  if (this->cs->valid && this->ts->valid)
  {
    if (this->ts->in_training)
    {
      /* Both, char sheet and training sheet is available and there is
       * a skill in training. We make sure the character sheet is up
       * to date by adding the previous level of the skill in training. */
      this->cs->add_char_skill(this->ts->skill, this->ts->to_level - 1);
      this->char_base_sp = this->cs->total_sp;

      /* Get the character skill in training. */
      this->training_cskill = this->cs->get_skill_for_id(this->ts->skill);
      this->char_group_base_sp = 0;

      if (this->training_cskill != 0)
      {
        /* Cache the amount of SP in the active skill group. */
        int group_id = this->training_cskill->details->group;
        for (std::size_t i = 0; i < this->cs->skills.size(); ++i)
        {
          ApiCharSheetSkill& cskill = this->cs->skills[i];
          if (cskill.details->group == group_id)
            this->char_group_base_sp += cskill.points;
        }
      }
      else
      {
        std::cout << "Warning: Skill in training (ID " << this->ts->skill
            << ") is unknown to " << this->cs->name << "!" << std::endl;
      }

      /* Update the live info. */
      this->update_live_info();
    }
    else if (this->ts->holds_completed)
    {
      /* The training sheet holds a skill that is already completed.
       * We can use this information to update the character sheet if
       * it does not yet have the new skill level. */
      this->cs->add_char_skill(this->ts->skill, this->ts->to_level);
      this->char_base_sp = this->cs->total_sp;
      this->training_cskill = 0;
    }
  }
  else
  {
    this->training_cskill = 0;
  }
}

/* ---------------------------------------------------------------- */

void
Character::update_live_info (void)
{
  if (!this->is_training())
    return;

  time_t evetime = EveTime::get_eve_time();
  time_t finish = this->ts->end_time_t;
  time_t diff = finish - evetime;

  //std::cout << "** Evetime: " << evetime << ", Finish: " << finish
  //    << ", time diff: " << diff << std::endl;

  /* Check if the skill is finished. */
  if (diff < 0)
  {
    this->skill_completed();
    return;
  }

  /* Update easy values first to get useful results even in case of errors. */
  unsigned int level_dest_sp = this->ts->dest_sp;
  double spps = (double)this->training_spph / 3600.0;

  this->training_remaining = diff;
  this->training_skill_sp = level_dest_sp - (unsigned int)((double)diff * spps);

  /* Check if the skill in training could be determined.
   * This may be NULL if the skill was not available in the skill tree. */
  if (this->training_skill == 0)
    return;

  /* Update training live values. */
  unsigned int level_start_sp = ApiCharSheet::calc_start_sp
      (this->ts->to_level - 1, this->training_skill->rank);
  unsigned int level_total_sp = level_dest_sp - level_start_sp;

  this->training_level_sp = this->training_skill_sp - level_start_sp;
  this->training_level_done = (double)this->training_level_sp
      / (double)level_total_sp;

  //std::cout << "** Start SP: " << level_start_sp << ", Dest SP: "
  //    << level_dest_sp << ", Level SP: " << level_total_sp << ", SP/s: "
  //    << spps << std::endl;

  if (!this->cs->valid)
    return;

  /* Check if the skill in training is available in the character.
   * This may be NULL if the skill was not available in the skill tree. */
  if (this->training_cskill == 0)
    return;

  /* Update character live values. */
  unsigned int basediff_sp = this->training_skill_sp
      - this->training_cskill->points;
  this->char_live_sp = this->char_base_sp + basediff_sp;
  this->char_group_live_sp = this->char_group_base_sp + basediff_sp;
}

/* ---------------------------------------------------------------- */

void
Character::skill_completed (void)
{
  /* Update the API info and reprocess to update the character. */
  this->ts->in_training = false;
  this->ts->holds_completed = true;
  this->process_api_data();

  /* At this point, the pointers to the skills are cleared. */
  this->training_spph = 0;
  this->training_level_sp = 0;
  this->training_skill_sp = 0;
  this->training_level_done = 0.0;
  this->training_remaining = 0;

  /* Emit the completed signal to notify listeners. */
  this->sig_skill_completed.emit();
}

/* ---------------------------------------------------------------- */

std::string
Character::get_char_name (void) const
{
  if (this->cs->valid)
    return this->cs->name;
  else
    return this->auth.char_id;
}

/* ---------------------------------------------------------------- */

std::string
Character::get_training_text (void) const
{
  if (this->is_training())
  {
    std::string to_level_str = Helpers::get_roman_from_int(this->ts->to_level);
    std::string skill_str;
    try
    {
      ApiSkillTreePtr tree = ApiSkillTree::request();
      ApiSkill const* skill = tree->get_skill_for_id(this->ts->skill);
      if (skill == 0)
        throw Exception();
      skill_str = skill->name;
    }
    catch (Exception& e)
    {
      /* This happens if the ID is not found. */
      skill_str = Helpers::get_string_from_int(this->ts->skill);
    }

    return skill_str + " " + to_level_str;
  }
  else
  {
    return "No skill in training!";
  }
}

/* ---------------------------------------------------------------- */

std::string
Character::get_remaining_text (bool slim) const
{
  if (!this->ts->valid)
    return "No training information!";

  if (!this->is_training())
    return "No skill in training!";

  return EveTime::get_string_for_timediff(this->training_remaining, slim);
}

/* ---------------------------------------------------------------- */

std::string
Character::get_summary_text (bool detailed)
{
  std::string ret;

  ret += this->get_char_name();
  ret += " - ";

  if (this->is_training())
  {
    ret += this->get_remaining_text(true);
    if (detailed)
    {
      ret += " - ";
      ret += this->get_training_text();
    }
  }
  else
    ret += "Not training!";

  return ret;
}

/* ---------------------------------------------------------------- */

void
Character::print_debug (void) const
{
  std::cout << "Character " << this->auth.char_id;
  if (this->cs->valid)
    std::cout << " (" << this->cs->name << ")";
  std::cout << std::endl;

  std::cout << "Training sheet valid: " << (this->ts->valid ?
      "Yes" : "No") << std::endl;
  if (this->ts->valid)
  {
    std::cout << "  Skill: " << this->training_skill->name << ", Group: "
        << this->training_skill->group << std::endl;
    std::cout << "  Live: Level SP: " << this->training_level_sp
        << ", Skill SP: " << this->training_skill_sp
        << " (" << training_level_done * 100.0 << "% done)" << std::endl;
    std::cout << "  Finished in " << EveTime::get_string_for_timediff
        (this->training_remaining, false) << std::endl;
    std::cout << std::endl;
  }

  std::cout << "Character sheet valid: " << (this->cs->valid ?
     "Yes" : "No") << std::endl;
  if (this->cs->valid)
  {
    std::cout << "Character: " << this->cs->name << " ("
        << this->char_base_sp << " SP)" << std::endl;
    std::cout << "  Live: Total SP: " << this->char_live_sp << std::endl;
  }

  if (this->cs->valid && this->ts->valid)
  {
    std::cout << "Character Live SP in Group: " << this->char_group_live_sp
        << " (" << this->char_group_base_sp << " base SP)" << std::endl;
  }

  std::cout << std::endl;
}
