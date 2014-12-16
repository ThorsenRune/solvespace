//-----------------------------------------------------------------------------
// The text-based browser window, used to view the structure of the model by
// groups and for other similar purposes.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"

//-----------------------------------------------------------------------------
// A navigation bar that always appears at the top of the window, with a
// link to bring us back home.
//-----------------------------------------------------------------------------
void TextWindow::ScreenHome(int link, uint32_t v) {
    SS.TW.GoToScreen(SCREEN_LIST_OF_GROUPS);
}
void TextWindow::ShowHeader(bool withNav) {
    ClearScreen();

    char cd[1024], cd2[1024];
    if(SS.GW.LockedInWorkplane()) {
        sprintf(cd, "in plane: ");
        strcpy(cd2, SK.GetEntity(SS.GW.ActiveWorkplane())->DescriptionString());
    } else {
        sprintf(cd, "drawing / constraining in 3d");
        strcpy(cd2, "");
    }

    // Navigation buttons
    if(withNav) {
        Printf(false, " %Fl%Lh%fhome%E   %Ft%s%E%s",
            (&TextWindow::ScreenHome), cd, cd2);
    } else {
        Printf(false, "        %Ft%s%E%s", cd, cd2);
    }

    // Leave space for the icons that are painted here.
    Printf(false, "");
    Printf(false, "");
}

//-----------------------------------------------------------------------------
// The screen that shows a list of every group in the sketch, with options
// to hide or show them, and to view them in detail. This is our home page.
//-----------------------------------------------------------------------------
void TextWindow::ScreenSelectGroup(int link, uint32_t v) {
    SS.TW.GoToScreen(SCREEN_GROUP_INFO);
    SS.TW.shown.group.v = v;
}
void TextWindow::ScreenToggleGroupShown(int link, uint32_t v) {
    hGroup hg = { v };
    Group *g = SK.GetGroup(hg);
    g->visible = !(g->visible);
    // If a group was just shown, then it might not have been generated
    // previously, so regenerate.
    SS.GenerateAll();
}
void TextWindow::ScreenShowGroupsSpecial(int link, uint32_t v) {
    int i;
    for(i = 0; i < SK.group.n; i++) {
        Group *g = &(SK.group.elem[i]);

        if(link == 's') {
            g->visible = true;
        } else {
            g->visible = false;
        }
    }
}
void TextWindow::ScreenActivateGroup(int link, uint32_t v) {//RT2014 Activate the group that the user clicked upon
    hGroup hg = { v };
    Group *g = SK.GetGroup(hg);
    g->visible = true;
    SS.GW.activeGroup.v = v;
    SK.GetGroup(SS.GW.activeGroup)->Activate();
    SS.GW.ClearSuper();
}
void TextWindow::ReportHowGroupSolved(hGroup hg) {
    SS.GW.ClearSuper();
    SS.TW.GoToScreen(SCREEN_GROUP_SOLVE_INFO);
    SS.TW.shown.group.v = hg.v;
    SS.later.showTW = true;
}
void TextWindow::ScreenHowGroupSolved(int link, uint32_t v) {
    if(SS.GW.activeGroup.v != v) {
        ScreenActivateGroup(link, v);
    }
    SS.TW.GoToScreen(SCREEN_GROUP_SOLVE_INFO);
    SS.TW.shown.group.v = v;
}
void TextWindow::ScreenShowConfiguration(int link, uint32_t v) {
    SS.TW.GoToScreen(SCREEN_CONFIGURATION);
}
void TextWindow::ScreenShowEditView(int link, uint32_t v) {
    SS.TW.GoToScreen(SCREEN_EDIT_VIEW);
}
void TextWindow::ScreenGoToWebsite(int link, uint32_t v) {
    OpenWebsite("http://solvespace.com/txtlink");
}
void TextWindow::ShowListOfGroups(void) {
    char radioTrue[]  = { ' ', (char)RADIO_TRUE,  ' ', 0 },
         radioFalse[] = { ' ', (char)RADIO_FALSE, ' ', 0 },
         checkTrue[]  = { ' ', (char)CHECK_TRUE,  ' ', 0 },
         checkFalse[] = { ' ', (char)CHECK_FALSE, ' ', 0 };

    Printf(true, "%Ft active");
    Printf(false, "%Ft    shown ok  group-name%E");
    int i;
    bool afterActive = false;
    for(i = 0; i < SK.group.n; i++) {
        Group *g = &(SK.group.elem[i]);
        char *s = g->DescriptionString();
        bool active = (g->h.v == SS.GW.activeGroup.v);
        bool shown = g->visible;
        bool ok = (g->solved.how == System::SOLVED_OKAY);
        bool ref = (g->h.v == Group::HGROUP_REFERENCES.v);
        Printf(false, "%Bp%Fd "
               "%Ft%s%Fb%D%f%Ll%s%E "
               "%Fb%s%D%f%Ll%s%E  "
               "%Fp%D%f%s%Ll%s%E  "
               "%Fl%Ll%D%f%s",
            // Alternate between light and dark backgrounds, for readability
                (i & 1) ? 'd' : 'a',
            // Link that activates the group
                ref ? "   " : "",
                g->h.v, (&TextWindow::ScreenActivateGroup),
                ref ? "" : (active ? radioTrue : radioFalse),
            // Link that hides or shows the group
                afterActive ? " - " : "",
                g->h.v, (&TextWindow::ScreenToggleGroupShown),
                afterActive ? "" : (shown ? checkTrue : checkFalse),
            // Link to the errors, if a problem occured while solving
            ok ? 's' : 'x', g->h.v, (&TextWindow::ScreenHowGroupSolved),
                ok ? "ok" : "",
                ok ? "" : "NO",
            // Link to a screen that gives more details on the group
            g->h.v, (&TextWindow::ScreenSelectGroup), s);

        if(active) afterActive = true;
		if (SS.revisionUnlockKey && REV1RT) afterActive = false;	//RT I like to see all group states
    }
	//RT START
	if (SS.revisionUnlockKey && REV1RT){
		if (SS.GW.activeGroup.v){
			Group *g = SK.group.FindByIdNoOops(SS.GW.activeGroup);
			if (g)
				Printf(true, "%Ft active (%d DOF)", g->solved.dof);
		}
	}
	//RT END

    Printf(true,  "  %Fl%Ls%fshow all%E / %Fl%Lh%fhide all%E",
        &(TextWindow::ScreenShowGroupsSpecial),
        &(TextWindow::ScreenShowGroupsSpecial));
    Printf(true,  "  %Fl%Ls%fline styles%E /"
                   " %Fl%Ls%fview%E /"
                   " %Fl%Ls%fconfiguration%E",
        &(TextWindow::ScreenShowListOfStyles),
        &(TextWindow::ScreenShowEditView),
        &(TextWindow::ScreenShowConfiguration));
}


//-----------------------------------------------------------------------------
// The screen that shows information about a specific group, and allows the
// user to edit various things about it.
//-----------------------------------------------------------------------------
void TextWindow::ScreenHoverConstraint(int link, uint32_t v) {
    if(!SS.GW.showConstraints) return;

    hConstraint hc = { v };
    Constraint *c = SK.GetConstraint(hc);
    if(c->group.v != SS.GW.activeGroup.v) {
        // Only constraints in the active group are visible
        return;
    }
    SS.GW.hover.Clear();
    SS.GW.hover.constraint = hc;
    SS.GW.hover.emphasized = true;
}
void TextWindow::ScreenHoverRequest(int link, uint32_t v) {
    SS.GW.hover.Clear();
    hRequest hr = { v };
    SS.GW.hover.entity = hr.entity(0);
    SS.GW.hover.emphasized = true;
}
void TextWindow::ScreenSelectConstraint(int link, uint32_t v) {
    SS.GW.ClearSelection();
    GraphicsWindow::Selection sel;
    ZERO(&sel);
    sel.constraint.v = v;
    SS.GW.selection.Add(&sel);
}
void TextWindow::ScreenSelectRequest(int link, uint32_t v) {
    SS.GW.ClearSelection();
    GraphicsWindow::Selection sel;
    ZERO(&sel);
    hRequest hr = { v };
    sel.entity = hr.entity(0);
    SS.GW.selection.Add(&sel);
}

void TextWindow::ScreenChangeGroupOption(int link, uint32_t v) {
    SS.UndoRemember();
    Group *g = SK.GetGroup(SS.TW.shown.group);

    switch(link) {
        case 's': g->subtype = Group::ONE_SIDED; break;
        case 'S': g->subtype = Group::TWO_SIDED; break;

        case 'k': g->skipFirst = true; break;
        case 'K': g->skipFirst = false; break;

        case 'c': g->meshCombine = v; break;

        case 'P': g->suppress = !(g->suppress); break;

        case 'r': g->relaxConstraints = !(g->relaxConstraints); break;

        case 'v': g->visible = !(g->visible); break;

        case 'd': g->allDimsReference = !(g->allDimsReference); break;

        case 'f': g->forceToMesh = !(g->forceToMesh); break;
    }

    SS.MarkGroupDirty(g->h);
    SS.GenerateAll();
    SS.GW.ClearSuper();
}

void TextWindow::ScreenColor(int link, uint32_t v) {
    SS.UndoRemember();

    Group *g = SK.GetGroup(SS.TW.shown.group);
    SS.TW.ShowEditControlWithColorPicker(v, 3, g->color);
    SS.TW.edit.meaning = EDIT_GROUP_COLOR;
}
void TextWindow::ScreenChangeExprA(int link, uint32_t v) {
    Group *g = SK.GetGroup(SS.TW.shown.group);

    // There's an extra line for the skipFirst parameter in one-sided groups.
    int r = (g->subtype == Group::ONE_SIDED) ? 16 : 14;

    char str[1024];
    sprintf(str, "%d", (int)g->valA);
    SS.TW.ShowEditControl(r, 10, str);
    SS.TW.edit.meaning = EDIT_TIMES_REPEATED;
    SS.TW.edit.group.v = v;
}
void TextWindow::ScreenChangeGroupName(int link, uint32_t v) {
    Group *g = SK.GetGroup(SS.TW.shown.group);
    SS.TW.ShowEditControl(7, 12, g->DescriptionString()+5);
    SS.TW.edit.meaning = EDIT_GROUP_NAME;
    SS.TW.edit.group.v = v;
}
void TextWindow::ScreenChangeGroupScale(int link, uint32_t v) {
    Group *g = SK.GetGroup(SS.TW.shown.group);
    char str[1024];
    switch(link) {
        case 'x':
            sprintf(str, "%.3f", g->scaleImported.x );
            SS.TW.ShowEditControl(16, 13, str);
            SS.TW.edit.meaning = EDIT_GROUP_SCALE;
            SS.pendingUserCommand=97701;
            SS.TW.edit.group.v = v;
            break;
        case 'y':
            sprintf(str, "%.3f", g->scaleImported.y );
            SS.TW.ShowEditControl(18, 13, str);
            SS.TW.edit.meaning = EDIT_GROUP_SCALE;
            SS.pendingUserCommand=97702;
            SS.TW.edit.group.v = v;
            break;
        case 'z':
            sprintf(str, "%.3f", g->scaleImported.z );
            SS.TW.ShowEditControl(20, 13, str);
            SS.TW.edit.meaning = EDIT_GROUP_SCALE;
            SS.pendingUserCommand=97703;
            SS.TW.edit.group.v = v;
            break;
        case 'a':                                   //Set all same
            sprintf(str, "%.3f", g->scaleImported.x);
            SS.TW.ShowEditControl(14, 13, str);
            SS.TW.edit.meaning = EDIT_GROUP_SCALE;
            SS.pendingUserCommand=97700;
            SS.TW.edit.group.v = v;
    }

}
void TextWindow::ScreenDeleteGroup(int link, uint32_t v) {
	SS.UndoRemember();

	hGroup hg = SS.TW.shown.group;

	if (SS.revisionUnlockKey && REV1RT) {
		/*	RT2014: disable the annoying condition for deleting active group.
		It makes more sense to actually require the group to be active before deleting
		therefore the logic of the following statement could be inverted			*/
		/*However its just annoying, so just let the user delete the group
		if (hg.v != SS.GW.activeGroup.v) {									//RT2014	Now you can only delete the currently active group
		Error("Please activate the group before deleting.");
		return;
		}
		*/
	}
	else {		//Original condition to disable deleting of active group
		if (hg.v == SS.GW.activeGroup.v) {
			Error("This group is currently active; activate a different group "
				"before proceeding.");
			return;
		}
	}
	SK.group.RemoveById(SS.TW.shown.group);
	// This is a major change, so let's re-solve everything.
	SS.TW.ClearSuper();
	SS.GW.ClearSuper();
	SS.GenerateAll(0, INT_MAX);
}

//RT***************************************New routines to move to adequate locations*****************************************
int TextWindow::txtConstraintNamesGet(Constraint *c, char **psNewString){   //RT2014 Supplementary description of constraints
	//Overwrite the description with named descriptions. The psNewString is the return value to call as  &oldString
//	static char ret[1024];		//RT Static because we return this string (?!)
	if (c->type == Constraint::EQUAL_LENGTH_LINES){
		Request *reqA = SK.GetRequest0(c->entityA.request());
		Request *reqB = SK.GetRequest0(c->entityB.request());
		if ((reqA) && (reqB))
 			sprintf(*psNewString, "%s == %s", reqA->str.str, reqB->str.str);
		return 1;
	}
	else if ((Constraint::PT_PT_DISTANCE) == c->type){
		//try{
		if (c->valA)	{		//Check if exists
			sprintf(*psNewString, "     %s", SS.MmToString(c->valA));			//Return the reference value
				return 1;
		}
			// Should never come here
		Error("Should never come here 313");
			Request *reqA = SK.GetRequest0(c->ptA.request());
			if (!(reqA))	{		//Check if exists
				sprintf(*psNewString, "     %s",   SS.MmToString(c->valA));		//Return the reference value
				return 1;
			}
			Request *reqB = SK.GetRequest0(c->ptB.request());
			if (!(reqB))
				return -1;
			Vector p0 = SK.GetEntity(c->ptA)->PointGetNum();
			Vector p1 = SK.GetEntity(c->ptB)->PointGetNum();
			sprintf(*psNewString, "%s == %s", reqA->str.str, SS.MmToString((p1.Minus(p0).Magnitude())));
			return 1;
			//	}
			//	catch (char *e) {
			//		Error("Problem PT_PT_DISTANCE: '%s'", e);
			//	}
	}
	else if ((Constraint::ANGLE) == c->type){
		try{
			Request *reqA = SK.GetRequest(c->entityA.request());
			Request *reqB = SK.GetRequest(c->entityB.request());
			Vector v0 = SK.GetEntity(c->entityA)->VectorGetNum();
			Vector v1 = SK.GetEntity(c->entityB)->VectorGetNum();
			v0 = v0.WithMagnitude(1);
			v1 = v1.WithMagnitude(1);
			double theta = acos(v0.Dot(v1));
			sprintf(*psNewString, "ang(%s, %s) = %.2f deg", reqA->str.str, reqB->str.str, theta * 180 / PI);
			return 1;
		}
		catch (char *e) {
			Error("Problem PT_PT_DISTANCE: '%s'", e);
		}
	}
	else if ((Constraint::EQUAL_RADIUS) == c->type){		//RT1215 Revised
		Entity *eA = SK.GetEntityNoOops(c->entityA);
		if (eA == NULL) Request *eA = SK.GetRequest(c->entityA.request());
		Entity *eB = SK.GetEntityNoOops(c->entityB);
		if (eB == NULL) Request *eB = SK.GetRequest(c->entityB.request());
		if ((eA)&&(eB))
			sprintf(*psNewString, "diam(%s) == diam(%s)", eA->str.str, eB->str.str);
		return 1;
	}
	else if ((Constraint::EQUAL_LINE_ARC_LEN) == c->type){
		Request *reqA = SK.GetRequest(c->entityA.request());
		Request *reqB = SK.GetRequest(c->entityB.request());
		if (Request::ARC_OF_CIRCLE == reqB->type){
			sprintf(*psNewString, "%s == (arc)%s", reqA->str.str, reqB->str.str);
		}
		else
			sprintf(*psNewString, "(arc)%s == %s", reqA->str.str, reqB->str.str);
		return 1;
	}
	else if ((Constraint::PT_ON_LINE) == c->type){
		Request *reqA = SK.GetRequest(c->entityA.request());
		sprintf(*psNewString, "pt X %s", reqA->str.str );
	}
	else if ((Constraint::PT_ON_CIRCLE) == c->type){
		Request *reqA = SK.GetRequest(c->entityA.request());
		sprintf(*psNewString, "pt X %s", reqA->str.str );
	}
 	else if ((Constraint::HORIZONTAL) == c->type){
		Request *reqA = SK.GetRequest(c->entityA.request());
		sprintf(*psNewString, "%s Horiz ", reqA->str.str);
	}
	else if ((Constraint::VERTICAL) == c->type){
		Request *reqA = SK.GetRequest(c->entityA.request());
		sprintf(*psNewString, "%s VERT", reqA->str.str);
	}
	else if ((Constraint::POINTS_COINCIDENT) == c->type) return 1;
	else if ((Constraint::COMMENT) == c->type) return 1;
return 0;				//No change
}
//RT*****************************   END OF New code *******************************
void TextWindow::ShowGroupInfo(void) {
    Group *g = SK.group.FindById(shown.group);
    const char *s = "???";

    if(shown.group.v == Group::HGROUP_REFERENCES.v) {
        Printf(true, "%FtGROUP  %E%s", g->DescriptionString());
        goto list_items;
    } else {
        Printf(true, "%FtGROUP  %E%s [%Fl%Ll%D%frename%E/%Fl%Ll%D%fdel%E]",
            g->DescriptionString(),
            g->h.v, &TextWindow::ScreenChangeGroupName,
            g->h.v, &TextWindow::ScreenDeleteGroup);
    }

    if(g->type == Group::LATHE) {
        Printf(true, " %Ftlathe plane sketch");
    } else if(g->type == Group::EXTRUDE || g->type == Group::ROTATE ||
              g->type == Group::TRANSLATE)
    {
        if(g->type == Group::EXTRUDE) {
            s = "extrude plane sketch";
        } else if(g->type == Group::TRANSLATE) {
            s = "translate original sketch";
        } else if(g->type == Group::ROTATE) {
            s = "rotate original sketch";
        }
        Printf(true, " %Ft%s%E", s);

        bool one = (g->subtype == Group::ONE_SIDED);
        Printf(false,
            "%Ba   %f%Ls%Fd%c one-sided%E  "
                  "%f%LS%Fd%c two-sided%E",
            &TextWindow::ScreenChangeGroupOption,
            one ? RADIO_TRUE : RADIO_FALSE,
            &TextWindow::ScreenChangeGroupOption,
            !one ? RADIO_TRUE : RADIO_FALSE);

        if(g->type == Group::ROTATE || g->type == Group::TRANSLATE) {
            if(g->subtype == Group::ONE_SIDED) {
                bool skip = g->skipFirst;
                Printf(false,
                   "%Bd   %Ftstart  %f%LK%Fd%c with original%E  "
                         "%f%Lk%Fd%c with copy #1%E",
                    &ScreenChangeGroupOption,
                    !skip ? RADIO_TRUE : RADIO_FALSE,
                    &ScreenChangeGroupOption,
                    skip ? RADIO_TRUE : RADIO_FALSE);
            }

            int times = (int)(g->valA);
            Printf(false, "%Bp   %Ftrepeat%E %d time%s %Fl%Ll%D%f[change]%E",
                (g->subtype == Group::ONE_SIDED) ? 'a' : 'd',
                times, times == 1 ? "" : "s",
                g->h.v, &TextWindow::ScreenChangeExprA);
        }
    } else if(g->type == Group::IMPORTED) {								//RT14126	set 3D scaling
        Printf(true, " %Ftimport geometry from file%E");
		Printf(false, "%Ba   '%s'    %Fl%Ll%f%D[Edit]%E", g->impFileRel, &TextWindow::editImportFileNameRT, 0);
        Printf(false, "%Bd   %Ftscaled by%E   %Fl%La%f%D[Uniform]%E",       &TextWindow::ScreenChangeGroupScale, g->h.v); //Make uniform scaling using x
        Printf(false, "%Bd   %Ft      X%E %# %Fl%Lx%f%D[change]%E", g->scaleImported.x, &TextWindow::ScreenChangeGroupScale, g->h.v);
        Printf(false, "%Bd   %Ft      Y%E %# %Fl%Ly%f%D[change]%E", g->scaleImported.y, &TextWindow::ScreenChangeGroupScale, g->h.v);
        Printf(false, "%Bd   %Ft      Z%E %# %Fl%Lz%f%D[change]%E", g->scaleImported.z, &TextWindow::ScreenChangeGroupScale, g->h.v);


        //    Printf(false, "%Bd   %Ftrotate%E    %@ degrees %Fl%Lr%f[change]%E",        shown.paste.theta*180/PI,        &ScreenChangePasteTransformed);
    } else if(g->type == Group::DRAWING_3D) {
        Printf(true, " %Ftsketch in 3d%E");
    } else if(g->type == Group::DRAWING_WORKPLANE) {
        Printf(true, " %Ftsketch in new workplane%E");
    } else {
        Printf(true, "???");
    }
    Printf(false, "");

    if(g->type == Group::EXTRUDE ||
       g->type == Group::LATHE ||
       g->type == Group::IMPORTED)
    {
        bool un   = (g->meshCombine == Group::COMBINE_AS_UNION);
        bool diff = (g->meshCombine == Group::COMBINE_AS_DIFFERENCE);
        bool asy  = (g->meshCombine == Group::COMBINE_AS_ASSEMBLE);
        bool asa  = (g->type == Group::IMPORTED);

        Printf(false, " %Ftsolid model as");
        Printf(false, "%Ba   %f%D%Lc%Fd%c union%E  "
                             "%f%D%Lc%Fd%c difference%E  "
                             "%f%D%Lc%Fd%c%s%E  ",
            &TextWindow::ScreenChangeGroupOption,
            Group::COMBINE_AS_UNION,
            un ? RADIO_TRUE : RADIO_FALSE,
            &TextWindow::ScreenChangeGroupOption,
            Group::COMBINE_AS_DIFFERENCE,
            diff ? RADIO_TRUE : RADIO_FALSE,
            &TextWindow::ScreenChangeGroupOption,
            Group::COMBINE_AS_ASSEMBLE,
            asa ? (asy ? RADIO_TRUE : RADIO_FALSE) : 0,
            asa ? " assemble" : "");

        if(g->type == Group::EXTRUDE ||
           g->type == Group::LATHE)
        {
            Printf(false,
                "%Bd   %Ftcolor %E%Bz  %Bd (%@, %@, %@) %f%D%Lf%Fl[change]%E",
                &g->color,
                g->color.redF(), g->color.greenF(), g->color.blueF(),
                ScreenColor, top[rows-1] + 2);
        } else if(g->type == Group::IMPORTED) {
            bool sup = g->suppress;
            Printf(false, "   %Fd%f%LP%c  suppress this group's solid model",
                &TextWindow::ScreenChangeGroupOption,
                g->suppress ? CHECK_TRUE : CHECK_FALSE);
        }

        Printf(false, "");
    }

    Printf(false, " %f%Lv%Fd%c  show entities from this group",
        &TextWindow::ScreenChangeGroupOption,
        g->visible ? CHECK_TRUE : CHECK_FALSE);

    Group *pg; pg = g->PreviousGroup();
    if(pg && pg->runningMesh.IsEmpty() && g->thisMesh.IsEmpty()) {
        Printf(false, " %f%Lf%Fd%c  force NURBS surfaces to triangle mesh",
            &TextWindow::ScreenChangeGroupOption,
            g->forceToMesh ? CHECK_TRUE : CHECK_FALSE);
    } else {
        Printf(false, " (model already forced to triangle mesh)");
    }

    Printf(true, " %f%Lr%Fd%c  relax constraints and dimensions",
        &TextWindow::ScreenChangeGroupOption,
        g->relaxConstraints ? CHECK_TRUE : CHECK_FALSE);

    Printf(false, " %f%Ld%Fd%c  treat all dimensions as reference",
        &TextWindow::ScreenChangeGroupOption,
        g->allDimsReference ? CHECK_TRUE : CHECK_FALSE);

    if(g->booleanFailed) {
        Printf(false, "");
        Printf(false, "The Boolean operation failed. It may be ");
        Printf(false, "possible to fix the problem by choosing ");
        Printf(false, "'force NURBS surfaces to triangle mesh'.");
    }

list_items:
	Printf(false, "");
	Printf(false, "%Ft requests in group");

	int i, a = 0;
	for (i = 0; i < SK.request.n; i++) {
		Request *r = &(SK.request.elem[i]);

		if (r->group.v == shown.group.v) {
			char *s = r->DescriptionString();
			if (*r->str.str != '\0')				//RT: No name, do nothing
			{
				if (r->construction)			//RT: Overwrite some descriptions with named
					sprintf(s, "%s  (Ref)", r->str.str);
				else
					sprintf(s, "%s", r->str.str);
			}
			Printf(false, "%Bp   %Fl%Ll%D%f%h%s%E",
				(a & 1) ? 'd' : 'a',
				r->h.v, (&TextWindow::ScreenSelectRequest),
				&(TextWindow::ScreenHoverRequest), s);
			a++;
		}
	}
	if (a == 0) Printf(false, "%Ba   (none)");

	a = 0;
	Printf(false, "");
	Printf(false, "%Ft constraints in group (%d DOF)", g->solved.dof);
	for (i = 0; i < SK.constraint.n; i++) {
		Constraint *c = &(SK.constraint.elem[i]);
		if (c->group.v == shown.group.v) {
			char *s = c->DescriptionString();
			if (SS.revisionUnlockKey && REV1RT)	txtConstraintNamesGet(c, &s); //RT Provide more info about constraints
			Printf(false, "%Bp   %Fl%Ll%D%f%h%s%E %s",
				(a & 1) ? 'd' : 'a',
				c->h.v, (&TextWindow::ScreenSelectConstraint),
				(&TextWindow::ScreenHoverConstraint), s,
				c->reference ? "(ref)" : "");
			a++;
		}
	}
	if (a == 0) Printf(false, "%Ba   (none)");
}


//-----------------------------------------------------------------------------
// The screen that's displayed when the sketch fails to solve. A report of
// what failed, and (if the problem is a singular Jacobian) a list of
// constraints that could be removed to fix it.
//-----------------------------------------------------------------------------
void TextWindow::ShowGroupSolveInfo(void) {
    Group *g = SK.group.FindById(shown.group);
    if(g->solved.how == System::SOLVED_OKAY) {
        // Go back to the default group info screen
        shown.screen = SCREEN_GROUP_INFO;
        Show();
        return;
    }

    Printf(true, "%FtGROUP   %E%s", g->DescriptionString());
    switch(g->solved.how) {
        case System::DIDNT_CONVERGE:
            Printf(true, "%FxSOLVE FAILED!%Fd no convergence");
            Printf(true, "the following constraints are unsatisfied");
            break;

        case System::SINGULAR_JACOBIAN:
            Printf(true, "%FxSOLVE FAILED!%Fd inconsistent system");
            Printf(true, "remove any one of these to fix it");
            break;

        case System::TOO_MANY_UNKNOWNS:
            Printf(true, "Too many unknowns in a single group!");
            return;
    }

	for (int i = 0; i < g->solved.remove.n; i++) {
		hConstraint hc = g->solved.remove.elem[i];
		Constraint *c = SK.constraint.FindByIdNoOops(hc);
		if (!c) continue;
		char *s = c->DescriptionString();
		if (SS.revisionUnlockKey && REV1RT) TextWindow::txtConstraintNamesGet(c, &s); //RT Provide additional info about constraints
		Printf(false, "%Bp   %Fl%Ll%D%f%h%s%E",
			(i & 1) ? 'd' : 'a',
			c->h.v, (&TextWindow::ScreenSelectConstraint),
			(&TextWindow::ScreenHoverConstraint),
			s);
	}

	Printf(true, "It may be possible to fix the problem ");
	Printf(false, "by selecting Edit -> Undo.");
}

//-----------------------------------------------------------------------------
// When we're stepping a dimension. User specifies the finish value, and
// how many steps to take in between current and finish, re-solving each
// time.
//-----------------------------------------------------------------------------
void TextWindow::ScreenStepDimFinish(int link, uint32_t v) {
    SS.TW.edit.meaning = EDIT_STEP_DIM_FINISH;
    char s[1024];
    if(SS.TW.shown.dimIsDistance) {
        strcpy(s, SS.MmToString(SS.TW.shown.dimFinish));
    } else {
        sprintf(s, "%.3f", SS.TW.shown.dimFinish);
    }
    SS.TW.ShowEditControl(12, 12, s);
}
void TextWindow::ScreenStepDimSteps(int link, uint32_t v) {
    char str[1024];
    sprintf(str, "%d", SS.TW.shown.dimSteps);
    SS.TW.edit.meaning = EDIT_STEP_DIM_STEPS;
    SS.TW.ShowEditControl(14, 12, str);
}
void TextWindow::ScreenStepDimGo(int link, uint32_t v) {
    hConstraint hc = SS.TW.shown.constraint;
    Constraint *c = SK.constraint.FindByIdNoOops(hc);
    if(c) {
        SS.UndoRemember();
        double start = c->valA, finish = SS.TW.shown.dimFinish;
        int i, n = SS.TW.shown.dimSteps;
        for(i = 1; i <= n; i++) {
            c = SK.GetConstraint(hc);
            c->valA = start + ((finish - start)*i)/n;
            SS.MarkGroupDirty(c->group);
            SS.GenerateAll();
            if(!SS.AllGroupsOkay()) {
                // Failed to solve, so quit
                break;
            }
            PaintGraphics();
        }
    }
    InvalidateGraphics();
    SS.TW.GoToScreen(SCREEN_LIST_OF_GROUPS);
}
void TextWindow::ShowStepDimension(void) {
    Constraint *c = SK.constraint.FindByIdNoOops(shown.constraint);
    if(!c) {
        shown.screen = SCREEN_LIST_OF_GROUPS;
        Show();
        return;
    }

    Printf(true, "%FtSTEP DIMENSION%E %s", c->DescriptionString());

    if(shown.dimIsDistance) {
        Printf(true,  "%Ba   %Ftstart%E    %s", SS.MmToString(c->valA));
        Printf(false, "%Bd   %Ftfinish%E   %s %Fl%Ll%f[change]%E",
            SS.MmToString(shown.dimFinish), &ScreenStepDimFinish);
    } else {
        Printf(true,  "%Ba   %Ftstart%E    %@", c->valA);
        Printf(false, "%Bd   %Ftfinish%E   %@ %Fl%Ll%f[change]%E",
            shown.dimFinish, &ScreenStepDimFinish);
    }
    Printf(false, "%Ba   %Ftsteps%E    %d %Fl%Ll%f%D[change]%E",
        shown.dimSteps, &ScreenStepDimSteps);

    Printf(true, " %Fl%Ll%fstep dimension now%E", &ScreenStepDimGo);

    Printf(true, "(or %Fl%Ll%fcancel operation%E)", &ScreenHome);
}

//-----------------------------------------------------------------------------
// When we're creating tangent arcs (as requests, not as some parametric
// thing). User gets to specify the radius, and whether the old untrimmed
// curves are kept or deleted.
//-----------------------------------------------------------------------------
void TextWindow::ScreenChangeTangentArc(int link, uint32_t v) {
    switch(link) {
        case 'r': {
            char str[1024];
            strcpy(str, SS.MmToString(SS.tangentArcRadius));
            SS.TW.edit.meaning = EDIT_TANGENT_ARC_RADIUS;
            SS.TW.ShowEditControl(12, 3, str);
            break;
        }

        case 'a': SS.tangentArcManual = !SS.tangentArcManual; break;
        case 'd': SS.tangentArcDeleteOld = !SS.tangentArcDeleteOld; break;
    }
}
void TextWindow::ShowTangentArc(void) {
    Printf(true, "%FtTANGENT ARC PARAMETERS%E");

    Printf(true,  "%Ft radius of created arc%E");
    if(SS.tangentArcManual) {
        Printf(false, "%Ba   %s %Fl%Lr%f[change]%E",
            SS.MmToString(SS.tangentArcRadius),
            &(TextWindow::ScreenChangeTangentArc));
    } else {
        Printf(false, "%Ba   automatic");
    }

    Printf(false, "");
    Printf(false, "  %Fd%f%La%c  choose radius automatically%E",
        &ScreenChangeTangentArc,
        !SS.tangentArcManual ? CHECK_TRUE : CHECK_FALSE);
    Printf(false, "  %Fd%f%Ld%c  delete original entities afterward%E",
        &ScreenChangeTangentArc,
        SS.tangentArcDeleteOld ? CHECK_TRUE : CHECK_FALSE);

    Printf(false, "");
    Printf(false, "To create a tangent arc at a point,");
    Printf(false, "select that point and then choose");
    Printf(false, "Sketch -> Tangent Arc at Point.");
    Printf(true, "(or %Fl%Ll%fback to home screen%E)", &ScreenHome);
}

//-----------------------------------------------------------------------------
// The edit control is visible, and the user just pressed enter.
//-----------------------------------------------------------------------------
void TextWindow::EditControlDone(const char *s) {
    edit.showAgain = false;

    switch(edit.meaning) {
        case EDIT_TIMES_REPEATED: {
            Expr *e = Expr::From(s, true);
            if(e) {
                SS.UndoRemember();

                double ev = e->Eval();
                if((int)ev < 1) {
                    Error("Can't repeat fewer than 1 time.");
                    break;
                }
                if((int)ev > 999) {
                    Error("Can't repeat more than 999 times.");
                    break;
                }

                Group *g = SK.GetGroup(edit.group);
                g->valA = ev;

                if(g->type == Group::ROTATE) {
                    int i, c = 0;
                    for(i = 0; i < SK.constraint.n; i++) {
                        if(SK.constraint.elem[i].group.v == g->h.v) c++;
                    }
                    // If the group does not contain any constraints, then
                    // set the numerical guess to space the copies uniformly
                    // over one rotation. Don't touch the guess if we're
                    // already constrained, because that would break
                    // convergence.
                    if(c == 0) {
                        double copies = (g->skipFirst) ? (ev + 1) : ev;
                        SK.GetParam(g->h.param(3))->val = PI/(2*copies);
                    }
                }

                SS.MarkGroupDirty(g->h);
                SS.later.generateAll = true;
            }
            break;
        }
        case EDIT_GROUP_NAME: {
            if(!StringAllPrintable(s) || !*s) {
                Error("Invalid characters. Allowed are: A-Z a-z 0-9 _ -");
            } else {
                SS.UndoRemember();

                Group *g = SK.GetGroup(edit.group);
                g->name.strcpy(s);
            }
            break;
        }
        case EDIT_GROUP_SCALE: {
            Expr *e = Expr::From(s, true);
            if(e) {
                double ev = e->Eval();
                if(fabs(ev) < 1e-6) {
                    Error("Scale cannot be zero.");
                } else {
                    Group *g = SK.GetGroup(edit.group);
                    // g->scaleImp = ev; //RT old code:
                    //RT Newcode
                    if (SS.pendingUserCommand==97701)
                      {g->scaleImported.x=ev;  }
                    else if (SS.pendingUserCommand==97702)
                      {g->scaleImported.y=ev;}
                    else if (SS.pendingUserCommand==97703)
                      {g->scaleImported.z=ev;}
                    else                            //Set same scale
                       {g->scaleImported.setSame(ev);}

                    SS.MarkGroupDirty(g->h);
                    SS.later.generateAll = true;
                    SS.pendingUserCommand=0;                    //Reset the pending command
                }
            }
            break;
        }
        case EDIT_GROUP_COLOR: {
            Vector rgb;
            if(sscanf(s, "%lf, %lf, %lf", &rgb.x, &rgb.y, &rgb.z)==3) {
                rgb = rgb.ClampWithin(0, 1);

                Group *g = SK.group.FindByIdNoOops(SS.TW.shown.group);
                if(!g) break;
                g->color = RGBf(rgb.x, rgb.y, rgb.z);

                SS.MarkGroupDirty(g->h);
                SS.later.generateAll = true;
                SS.GW.ClearSuper();
            } else {
                Error("Bad format: specify color as r, g, b");
            }
            break;
        }
        case EDIT_TTF_TEXT: {
            SS.UndoRemember();
            Request *r = SK.request.FindByIdNoOops(edit.request);
            if(r) {
                r->str.strcpy(s);
                SS.MarkGroupDirty(r->group);
                SS.later.generateAll = true;
            }
            break;
        }
        case EDIT_STEP_DIM_FINISH: {
            Expr *e = Expr::From(s, true);
            if(!e) {
                break;
            }
            if(shown.dimIsDistance) {
                shown.dimFinish = SS.ExprToMm(e);
            } else {
                shown.dimFinish = e->Eval();
            }
            break;
        }
        case EDIT_STEP_DIM_STEPS:
            shown.dimSteps = min(300, max(1, atoi(s)));
            break;

        case EDIT_TANGENT_ARC_RADIUS: {
            Expr *e = Expr::From(s, true);
            if(!e) break;
            if(e->Eval() < LENGTH_EPS) {
                Error("Radius cannot be zero or negative.");
                break;
            }
            SS.tangentArcRadius = SS.ExprToMm(e);
            break;
        }

        default: {
            int cnt = 0;
            if(EditControlDoneForStyles(s))         cnt++;
            if(EditControlDoneForConfiguration(s))  cnt++;
            if(EditControlDoneForPaste(s))          cnt++;
            if(EditControlDoneForView(s))           cnt++;
            if(cnt > 1) {
                // The identifiers were somehow assigned not uniquely?
                oops();
            }
            break;
        }
    }
    InvalidateGraphics();
    SS.later.showTW = true;

    if(!edit.showAgain) {
        HideEditControl();
        edit.meaning = EDIT_NOTHING;
    }
}

//RT2014 Dec. New methods for the GUI

void TextWindow::editImportFileNameRT(int link, uint32_t v) {	//RT: Let the user change the imported file
	//Params: two dummies to conform with e.g.&TextWindow::ScreenEditTtfText, use 0,0
	Group *g = SK.group.FindById(SS.GW.activeGroup);
	if (g->type = Group::IMPORTED) {
		if (!GetOpenFile(g->impFile, SLVS_EXT, SLVS_PATTERN)) return;
		strcpy(g->impFileRel, g->impFile);					//Copy the full filename
		MakePathRelative(SS.saveFile, g->impFileRel);		//Make the copy relative to current path
		Message("New Relative Filename: %s", g->impFileRel);
		SS.AfterNewFile();									//Regenerate all
	}
	else
		Message("No imported file");
	return;
}
//RT2014	end of new methods for GUI
