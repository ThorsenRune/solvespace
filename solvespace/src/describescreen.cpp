//-----------------------------------------------------------------------------
// The screens when an entity is selected, that show some description of it--
// endpoints of the lines, diameter of the circle, etc.
//
// Copyright 2008-2013 Jonathan Westhues.
//-----------------------------------------------------------------------------
#include "solvespace.h"

void TextWindow::ScreenUnselectAll(int link, uint32_t v) {
    GraphicsWindow::MenuEdit(GraphicsWindow::MNU_UNSELECT_ALL);
}

void TextWindow::ScreenEditTtfText(int link, uint32_t v) {          //RTc user has activated the edit link
    hRequest hr = { v };
	Request *r;
	if (SS.revisionUnlockKey && REV1RT){					//RT No crash code


		r = SK.GetRequest0(hr);						//Get the request
		if (r == NULL) {
			Error("Error in 'ScreenEditTtfText' caused by invalid handle to inexistent Request element");
			return;				//RT: Why let it crash here
		}
	}
	else{
		r = SK.GetRequest(hr);				//RTc: Original code, die on null pointer
	}
    SS.TW.ShowEditControl(13, 10, r->str.str);
	SS.TW.edit.meaning = EDIT_TTF_TEXT;
    SS.TW.edit.request = hr;
}
//RT New code
void TextWindow::ScreenEditTextRT(int link, uint32_t v) {          //RTc user has activated the edit
    SS.TW.edit.UId = 0;
    SS.TW.edit.meaning = EDIT_TTF_TEXT;
    if (link=='c'){                                 //Format for Print Lc%f%D link=c f=this function D=v identifier
        hConstraint h = { v };
        Constraint* c1 = SK.constraint.FindByIdNoOops(h);
        if (c1) {
            SS.TW.ShowEditControl(14, 10, c1->comment.str);
            SS.TW.edit.editAction=1216;         //RT to be substituted by a enumeration constant EDIT_CONSTRAINTNAME
            SS.TW.edit.UId = v;
        }
    }
    else if (link!='c') {                       //Message("other link sent ");
        Request* r1 = SK.request.getByUId(v);
        if (r1) {
            SS.TW.ShowEditControl(13, 10, r1->str.str);
            SS.TW.edit.editAction=1217;
            SS.TW.edit.UId = v;
        }
    }
}
//RT End code

#define gs (SS.GW.gs)
void TextWindow::ScreenSetTtfFont(int link, uint32_t v) {
    int i = (int)v;
    if(i < 0) return;
    if(i >= SS.fonts.l.n) return;

    SS.GW.GroupSelection();
    if(gs.entities != 1 || gs.n != 1) return;

    Entity *e = SK.entity.FindByIdNoOops(gs.entity[0]);
    if(!e || e->type != Entity::TTF_TEXT || !e->h.isFromRequest()) return;

    Request *r = SK.request.FindByIdNoOops(e->h.request());
    if(!r) return;

    SS.UndoRemember();
    r->font.strcpy(SS.fonts.l.elem[i].FontFileBaseName());
    SS.MarkGroupDirty(r->group);
    SS.later.generateAll = true;
    SS.later.showTW = true;
}
//RT START PROTOTYPE CODE
void TextWindow::txtEntityDescription_RT(Entity * e){
	Request* req1 = NULL;
// This adds an edit name link to the elements
	if ((e->h.isFromRequest()))	{		//The Entity is constructed therefore volatile, get the request that generates it
		req1 = SK.request.FindByIdNoOops(e->h.request());
		if (!(req1)) Error("Err:1216 expected request");		//RT Now we should have the request
		if (req1->type == Request::LINE_SEGMENT){
			//RT Let user edit the segment name
			if (*req1->str.str == '\0') {		//RT Make sure that line has a name
				sprintf(req1->str.str, "L%d", req1->h.v);	//RT Use handle number as identifier
			}
			Printf(false, "'%Fi%s%E' %Fl%Ll%f%D[Edit]%E",
				req1->str.str, &TextWindow::ScreenEditTtfText, req1->h);
			return;
		}
	}
	// No request has been found
	if (e->type == Entity::LINE_SEGMENT){ //These are to be rewritten
		//RT Let user edit the segment name
		Request *rLn1 = SK.GetRequest0(e->h.request());
		if (e->h.isFromRequest()==false) return;
		if (!rLn1) ERRMSG_RT();
		if (*rLn1->str.str == '\0') {		//RT Make sure that line has a name
			sprintf(rLn1->str.str, "L%d", rLn1->h.v);	//RT Use handle number as identifier
		}
		Printf(false, "'%Fi%s%E' %Fl%Ll%f%D[Edit]%E",
			rLn1->str.str, &TextWindow::ScreenEditTtfText, e->h.request());
	}
	else if (e->type == Entity::LINE_SEGMENT){
		Request *rLn1 = SK.GetRequest(e->h.request());
		if (*rLn1->str.str == '\0') {		//RT Make sure that line has a name
			sprintf(rLn1->str.str, "C%d", rLn1->h.v);	//RT Use handle number as identifier
		}
		TextWindow::Printf(false, "'%Fi%s%E' %Fl%Ll%f%D[Edit]%E",
			rLn1->str.str, &TextWindow::ScreenEditTtfText, e->h.request());
	}
	else if (e->type == Entity::ARC_OF_CIRCLE){
			Request *rLn1 = SK.GetRequest(e->h.request());
			if (*rLn1->str.str == '\0') {		//RT Make sure that line has a name
				sprintf(rLn1->str.str, "A%d", rLn1->h.v);	//RT Use handle number as identifier
			}
			TextWindow::Printf(false, "'%Fi%s%E' %Fl%Ll%f%D[Edit]%E",
				rLn1->str.str, &TextWindow::ScreenEditTextRT, e->h.request());
	}
	else if (e->type == Entity::CIRCLE){
		Request *rLn1 = SK.GetRequest0(e->h.request());
		if (rLn1 == NULL) {												// It its not a request then get it by its name
			Request* req1 = Request::getRequestByName(e->str.str);
			if (req1 == NULL)
				TextWindow::Printf(false, "'%Fi%s",					e->str.str);
			else
				TextWindow::Printf(false, "'%Fi%s%E' %Fl%Ll%f%D[Edit]%E",
					req1->str.str, &TextWindow::ScreenEditTtfText, req1->h.v);
		}
		else
		{
		if (*rLn1->str.str == '\0') {		//RT Make sure that line has a name
			sprintf(rLn1->str.str, "C%d", rLn1->h.v);	//RT Use handle number as identifier
			}
			TextWindow::Printf(false, "'%Fi%s%E' %Fl%Ll%f%D[Edit]%E",
			rLn1->str.str, &TextWindow::ScreenEditTtfText, e->h.request());
		}
	}
	else if (e->type == Entity::WORKPLANE){
		//?! The workplane cannot return request
	}
}
//RT  ************************END of prototyping code*************************************************

void TextWindow::DescribeSelection(void) {
    Entity *e=NULL;
    Vector p;
    int i;
    Printf(false, "");

    if(gs.n == 1 && (gs.points == 1 || gs.entities == 1)) {
        e = SK.GetEntity(gs.points == 1 ? gs.point[0] : gs.entity[0]);

#define COSTR(p) \
    SS.MmToString((p).x), SS.MmToString((p).y), SS.MmToString((p).z)
#define PT_AS_STR "(%Fi%s%E, %Fi%s%E, %Fi%s%E)"
#define PT_AS_NUM "(%Fi%3%E, %Fi%3%E, %Fi%3%E)"
        switch(e->type) {
            case Entity::POINT_IN_3D:
            case Entity::POINT_IN_2D:
            case Entity::POINT_N_TRANS:
            case Entity::POINT_N_ROT_TRANS:
            case Entity::POINT_N_COPY:
            case Entity::POINT_N_ROT_AA:
                p = e->PointGetNum();
				Printf(false, "%FtPOINT%E at " PT_AS_STR, COSTR(p));
				if (SS.revisionUnlockKey && REV1RT)
					txtEntityDescription_RT(e);	//RT Let user edit the segment name
                break;

            case Entity::NORMAL_IN_3D:
            case Entity::NORMAL_IN_2D:
            case Entity::NORMAL_N_COPY:
            case Entity::NORMAL_N_ROT:
            case Entity::NORMAL_N_ROT_AA: {
                Quaternion q = e->NormalGetNum();
                p = q.RotationN();
                Printf(false, "%FtNORMAL / COORDINATE SYSTEM%E");
                Printf(true,  "  basis n = " PT_AS_NUM, CO(p));
                p = q.RotationU();
                Printf(false, "        u = " PT_AS_NUM, CO(p));
                p = q.RotationV();
                Printf(false, "        v = " PT_AS_NUM, CO(p));
                break;
            }
            case Entity::WORKPLANE: {
                p = SK.GetEntity(e->point[0])->PointGetNum();
                Printf(false, "%FtWORKPLANE%E");
				if (SS.revisionUnlockKey && REV1RT)
					txtEntityDescription_RT(e);	//RT Let user edit the segment name
				Printf(true, "   origin = " PT_AS_STR, COSTR(p));
                Quaternion q = e->Normal()->NormalGetNum();
                p = q.RotationN();
                Printf(true, "   normal = " PT_AS_NUM, CO(p));
                break;
            }
			// Entity e == SK.GetEntity selection
			case Entity::LINE_SEGMENT: {		//RT e->type==LINE_SEGMENT
			Vector p0 = SK.GetEntity(e->point[0])->PointGetNum();
				p = p0;
				Printf(false, "%FtLINE SEGMENT%E");
				if (SS.revisionUnlockKey && REV1RT)
					txtEntityDescription_RT(e);	//RT Let user edit the segment name
                Printf(true,  "   thru " PT_AS_STR, COSTR(p));
                Vector p1 = SK.GetEntity(e->point[1])->PointGetNum();
                p = p1;
                Printf(false, "        " PT_AS_STR, COSTR(p));
                Printf(true,  "   len = %Fi%s%E",
                    SS.MmToString((p1.Minus(p0).Magnitude())));
                break;
            }
            case Entity::CUBIC_PERIODIC:
            case Entity::CUBIC:
                int pts;
                if(e->type == Entity::CUBIC_PERIODIC) {
                    Printf(false, "%FtPERIODIC C2 CUBIC SPLINE%E");
                    pts = (3 + e->extraPoints);
                } else if(e->extraPoints > 0) {
                    Printf(false, "%FtINTERPOLATING C2 CUBIC SPLINE%E");
                    pts = (4 + e->extraPoints);
                } else {
                    Printf(false, "%FtCUBIC BEZIER CURVE%E");
                    pts = 4;
                }
                for(i = 0; i < pts; i++) {
                    p = SK.GetEntity(e->point[i])->PointGetNum();
                    Printf((i==0), "   p%d = " PT_AS_STR, i, COSTR(p));
                }
                break;

            case Entity::ARC_OF_CIRCLE: {
                Printf(false, "%FtARC OF A CIRCLE%E");
				if (SS.revisionUnlockKey && REV1RT)
					txtEntityDescription_RT(e);	//RT Let user edit the segment name
                p = SK.GetEntity(e->point[0])->PointGetNum();
                Printf(true,  "     center = " PT_AS_STR, COSTR(p));
                p = SK.GetEntity(e->point[1])->PointGetNum();
                Printf(true,  "  endpoints = " PT_AS_STR, COSTR(p));
                p = SK.GetEntity(e->point[2])->PointGetNum();
                Printf(false, "              " PT_AS_STR, COSTR(p));
                double r = e->CircleGetRadiusNum();
                Printf(true, "   diameter =  %Fi%s", SS.MmToString(r*2));
                Printf(false, "     radius =  %Fi%s", SS.MmToString(r));
                double thetas, thetaf, dtheta;
                e->ArcGetAngles(&thetas, &thetaf, &dtheta);
                Printf(false, "    arc len =  %Fi%s", SS.MmToString(dtheta*r));
                break;
            }
            case Entity::CIRCLE: {
                Printf(false, "%FtCIRCLE%E");
				if (SS.revisionUnlockKey && REV1RT)
					txtEntityDescription_RT(e);	//RT Let user edit the segment name
                p = SK.GetEntity(e->point[0])->PointGetNum();
                Printf(true,  "     center = " PT_AS_STR, COSTR(p));
                double r = e->CircleGetRadiusNum();
                Printf(true,  "   diameter =  %Fi%s", SS.MmToString(r*2));
                Printf(false, "     radius =  %Fi%s", SS.MmToString(r));
                break;
            }
            case Entity::FACE_NORMAL_PT:
            case Entity::FACE_XPROD:
            case Entity::FACE_N_ROT_TRANS:
            case Entity::FACE_N_ROT_AA:
            case Entity::FACE_N_TRANS:
                Printf(false, "%FtPLANE FACE%E");
                p = e->FaceGetNormalNum();
                Printf(true,  "   normal = " PT_AS_NUM, CO(p));
                p = e->FaceGetPointNum();
                Printf(false, "     thru = " PT_AS_STR, COSTR(p));
                break;

            case Entity::TTF_TEXT: {
                Printf(false, "%FtTRUETYPE FONT TEXT%E");
                Printf(true, "  font = '%Fi%s%E'", e->font.str);
                if(e->h.isFromRequest()) {
                    Printf(false, "  text = '%Fi%s%E' %Fl%Ll%f%D[change]%E",
                        e->str.str, &ScreenEditTtfText, e->h.request());
                    Printf(true, "  select new font");
                    SS.fonts.LoadAll();
                    int i;
                    for(i = 0; i < SS.fonts.l.n; i++) {
                        TtfFont *tf = &(SS.fonts.l.elem[i]);
                        if(strcmp(e->font.str, tf->FontFileBaseName())==0) {
                            Printf(false, "%Bp    %s",
                                (i & 1) ? 'd' : 'a',
                                tf->name.str);
                        } else {
                            Printf(false, "%Bp    %f%D%Fl%Ll%s%E%Bp",
                                (i & 1) ? 'd' : 'a',
                                &ScreenSetTtfFont, i,
                                tf->name.str,
                                (i & 1) ? 'd' : 'a');
                        }
                    }
                } else {
                    Printf(false, "  text = '%Fi%s%E'", e->str.str);
                }
                break;
            }

            default:
                Printf(true, "%Ft?? ENTITY%E");
                break;
        }

        Group *g = SK.GetGroup(e->group);
        Printf(false, "");
        Printf(false, "%FtIN GROUP%E      %s", g->DescriptionString());
        if(e->workplane.v == Entity::FREE_IN_3D.v) {
            Printf(false, "%FtNOT LOCKED IN WORKPLANE%E");
        } else {
            Entity *w = SK.GetEntity(e->workplane);
            Printf(false, "%FtIN WORKPLANE%E  %s", w->DescriptionString());
        }
        if(e->style.v) {
            Style *s = Style::Get(e->style);
            Printf(false, "%FtIN STYLE%E      %s", s->DescriptionString());
        } else {
            Printf(false, "%FtIN STYLE%E      none");
        }
        if(e->construction) {
            Printf(false, "%FtCONSTRUCTION");
        }
		if (SS.revisionUnlockKey && REV1RT){	// Is this a user drawn element
			if (e->h.isFromRequest() == false) {
				Printf(false, "%FtGENERATED");
			}
		}
    } else if(gs.n == 2 && gs.points == 2) {
        Printf(false, "%FtTWO POINTS");
        Vector p0 = SK.GetEntity(gs.point[0])->PointGetNum();
        Printf(true,  "   at " PT_AS_STR, COSTR(p0));
        Vector p1 = SK.GetEntity(gs.point[1])->PointGetNum();
        Printf(false, "      " PT_AS_STR, COSTR(p1));
        double d = (p1.Minus(p0)).Magnitude();
        Printf(true, "  d = %Fi%s", SS.MmToString(d));
    } else if(gs.n == 2 && gs.faces == 1 && gs.points == 1) {
        Printf(false, "%FtA POINT AND A PLANE FACE");
        Vector pt = SK.GetEntity(gs.point[0])->PointGetNum();
        Printf(true,  "        point = " PT_AS_STR, COSTR(pt));
        Vector n = SK.GetEntity(gs.face[0])->FaceGetNormalNum();
        Printf(true,  " plane normal = " PT_AS_NUM, CO(n));
        Vector pl = SK.GetEntity(gs.face[0])->FaceGetPointNum();
        Printf(false, "   plane thru = " PT_AS_STR, COSTR(pl));
        double dd = n.Dot(pl) - n.Dot(pt);
        Printf(true,  "     distance = %Fi%s", SS.MmToString(dd));
    } else if(gs.n == 3 && gs.points == 2 && gs.vectors == 1) {
        Printf(false, "%FtTWO POINTS AND A VECTOR");
        Vector p0 = SK.GetEntity(gs.point[0])->PointGetNum();
        Printf(true,  "  pointA = " PT_AS_STR, COSTR(p0));
        Vector p1 = SK.GetEntity(gs.point[1])->PointGetNum();
        Printf(false, "  pointB = " PT_AS_STR, COSTR(p1));
        Vector v  = SK.GetEntity(gs.vector[0])->VectorGetNum();
        v = v.WithMagnitude(1);
        Printf(true,  "  vector = " PT_AS_NUM, CO(v));
        double d = (p1.Minus(p0)).Dot(v);
        Printf(true,  "  proj_d = %Fi%s", SS.MmToString(d));
    } else if(gs.n == 2 && gs.lineSegments == 1 && gs.points == 1) {
        Entity *ln = SK.GetEntity(gs.entity[0]);
        Vector lp0 = SK.GetEntity(ln->point[0])->PointGetNum(),
               lp1 = SK.GetEntity(ln->point[1])->PointGetNum();
        Printf(false, "%FtLINE SEGMENT AND POINT%E");
        Printf(true,  "   ln thru " PT_AS_STR, COSTR(lp0));
        Printf(false, "           " PT_AS_STR, COSTR(lp1));
        Vector pp = SK.GetEntity(gs.point[0])->PointGetNum();
        Printf(true,  "     point " PT_AS_STR, COSTR(pp));
        Printf(true,  " pt-ln distance = %Fi%s%E",
            SS.MmToString(pp.DistanceToLine(lp0, lp1.Minus(lp0))));
	}
	else if (gs.n == 2 && gs.vectors == 2) {
		Printf(false, "%FtTWO VECTORS");

		Vector v0 = SK.GetEntity(gs.entity[0])->VectorGetNum();
		Vector v1 = SK.GetEntity(gs.entity[1])->VectorGetNum();
		v0 = v0.WithMagnitude(1);
		v1 = v1.WithMagnitude(1);

		if (!(SS.solveOptions & EDIT_ENABLENAMING)){//RT Pevious stuff
			Printf(true, "  vectorA = " PT_AS_NUM, CO(v0));
			Printf(false, "  vectorB = " PT_AS_NUM, CO(v1));
		}
		else 							//Name the vectors
		{	//RT named lines Substituting above with names
			Request* rLn1 = SK.GetRequest0(gs.entity[0].request());	//Get object containing the strings
			if (rLn1){							// Only if the request exists
				Printf(false, "Line %s  = " PT_AS_NUM, rLn1->str.str, CO(v0));
			}
			else
				Printf(false, "Generated  = " PT_AS_NUM, CO(v0));
			Request* rLn2 = SK.GetRequest0(gs.entity[1].request());	//Return object containing the strings
			if (rLn2)							// Only if the request exists
				Printf(false, "Line %s  = " PT_AS_NUM, rLn2->str.str, CO(v1));
			else
				Printf(false, "Generated  = " PT_AS_NUM , CO(v1));
		}									//RT*****************END showing named lines
        double theta = acos(v0.Dot(v1));
        Printf(true,  "    angle = %Fi%2%E degrees", theta*180/PI);
        while(theta < PI/2) theta += PI;
        while(theta > PI/2) theta -= PI;
        Printf(false, " or angle = %Fi%2%E (mod 180)", theta*180/PI);
    } else if(gs.n == 2 && gs.faces == 2) {
        Printf(false, "%FtTWO PLANE FACES");

        Vector n0 = SK.GetEntity(gs.face[0])->FaceGetNormalNum();
        Printf(true,  " planeA normal = " PT_AS_NUM, CO(n0));
        Vector p0 = SK.GetEntity(gs.face[0])->FaceGetPointNum();
        Printf(false, "   planeA thru = " PT_AS_STR, COSTR(p0));

        Vector n1 = SK.GetEntity(gs.face[1])->FaceGetNormalNum();
        Printf(true,  " planeB normal = " PT_AS_NUM, CO(n1));
        Vector p1 = SK.GetEntity(gs.face[1])->FaceGetPointNum();
        Printf(false, "   planeB thru = " PT_AS_STR, COSTR(p1));

        double theta = acos(n0.Dot(n1));
        Printf(true,  "         angle = %Fi%2%E degrees", theta*180/PI);
        while(theta < PI/2) theta += PI;
        while(theta > PI/2) theta -= PI;
        Printf(false, "      or angle = %Fi%2%E (mod 180)", theta*180/PI);

        if(fabs(theta) < 0.01) {
            double d = (p1.Minus(p0)).Dot(n0);
            Printf(true,  "      distance = %Fi%s", SS.MmToString(d));
        }
    } else if(gs.n == 0 && gs.stylables > 0) {
        Printf(false, "%FtSELECTED:%E comment text");
    } else if(gs.n == 0 && gs.constraints == 1) {
        Printf(false, "%FtSELECTED:%E %s",
            SK.GetConstraint(gs.constraint[0])->DescriptionString());
		if (SS.solveOptions & EDIT_ENABLENAMING){
			Constraint *c = SK.GetConstraint(gs.constraint[0]);
			char *s = c->DescriptionString();
            Printf(true, "'%Fi%s%E' %Fl%Lc%f%D[Edit]%E",                    //Link='c'
				c->comment.str, &TextWindow::ScreenEditTextRT, c->h);
			TextWindow::txtConstraintNamesGet(c, &s); //RT Provide more info about constraints
			Printf(false, "%Ft%E %s", s);
		}
    } else {
        int n = SS.GW.selection.n;
        Printf(false, "%FtSELECTED:%E %d item%s", n, n == 1 ? "" : "s");
    }

    if(shown.screen == SCREEN_STYLE_INFO &&
       shown.style.v >= Style::FIRST_CUSTOM && gs.stylables > 0)
    {
        // If we are showing a screen for a particular style, then offer the
        // option to assign our selected entities to that style.
        Style *s = Style::Get(shown.style);
        Printf(true, "%Fl%D%f%Ll(assign to style %s)%E",
            shown.style.v,
            &ScreenAssignSelectionToStyle,
            s->DescriptionString());
    }
    // If any of the selected entities have an assigned style, then offer
    // the option to remove that style.
    bool styleAssigned = false;
    for(i = 0; i < gs.entities; i++) {
        Entity *e = SK.GetEntity(gs.entity[i]);
        if(e->style.v != 0) {
            styleAssigned = true;
        }
    }
    for(i = 0; i < gs.constraints; i++) {
        Constraint *c = SK.GetConstraint(gs.constraint[i]);
        if(c->type == Constraint::COMMENT && c->disp.style.v != 0) {
            styleAssigned = true;
        }
    }
    if(styleAssigned) {
        Printf(true, "%Fl%D%f%Ll(remove assigned style)%E",
            0,
            &ScreenAssignSelectionToStyle);
    }
	if (SS.revisionUnlockKey && REV1RT){
		if (e){
			Group *g = SK.GetGroup(e->group);
			Printf(true,
				"%Fl%Ll%D%f%s",
				//%D=data to pass to function
				//%f =function
				//%s=function description
				g->h.v, (&TextWindow::ScreenActivateGroup), "Activate group");
		}
		else
		{
			// more than one entity was selected
		}
	}

    Printf(true, "%Fl%f%Ll(unselect all)%E", &TextWindow::ScreenUnselectAll);
}

void TextWindow::GoToScreen(int screen) {
    shown.screen = screen;
}

