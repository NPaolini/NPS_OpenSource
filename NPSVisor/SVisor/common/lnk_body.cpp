//--------- lnk_body.cpp -----------------------------------------------------
//----------------------------------------------------------------------------
#include "precHeader.h"
//----------------------------------------------------------------------------
#include "lnk_body.h"
//----------------------------------------------------------------------------
// funzioni per creazione Body

extern fnBody getEmpty;
extern fnBody getMainMenu;

extern fnBody getRecipe;
extern fnBody getRecipeDan;
extern fnBody getRecipeRow;
//extern fnBody getManut;
extern fnBody getLGraph2;
extern fnBody get_saveData_Body;

// struttura per collegamento id-funzione
struct lnk
{
  int id;
  fnBody *fn;
};
static const lnk Body[] =
{
  // body predefiniti, da non modificare
  { IDD_MAIN_MENU, getMainMenu },
  { IDD_EMPTY,     getEmpty  },
  { IDD_RECIPE,     getRecipe },
  { IDD_RECIPE + 1,     getRecipeDan },
  { IDD_RECIPE_ROW,  getRecipeRow },
//  { IDD_MANUTENZ,  getManut    },
  { IDD_LGRAPH2,    getLGraph2 },
  { IDD_SAVE_DATA,  get_saveData_Body },
  // seguono body da definire

};

P_Body *getBody(int idNewBody, PWin* parent, int idPar)
{
  // Ricerca l'Id nell'array di lnk.
  // Se lo trova, ne invoca la funzione
  for(int i = 0; i < SIZE_A(Body); ++i)
    if(Body[i].id == idNewBody)
      return Body[i].fn(idPar, parent);
  return 0;
}

