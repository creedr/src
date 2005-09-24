/* Convert shots to CMPs for regular 2-D geometry. */
/*
  Copyright (C) 2004 University of Texas at Austin
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <string.h>

#include <rsf.h>

int main(int argc, char* argv[])
{
    int type, esize;
    off_t pos;
    bool sign;
    int   ns,    ny,    nh, nh2, nt;
    int   is,    iy,    ih, ih2=0, *mask;
    float os,ds, oy,dy, oh,dh;
    char *trace, *zero;
    sf_file in, out, msk;

    sf_init(argc,argv);
    in  = sf_input ( "in");
    out = sf_output("out");

    if (!sf_histint  (in,"n1",&nt)) sf_error("No n1= in input");

    if (!sf_histint  (in,"n2",&nh)) sf_error("No n2= in input");
    if (!sf_histfloat(in,"o2",&oh)) sf_error("No o2= in input");
    if (!sf_histfloat(in,"d2",&dh)) sf_error("No d2= in input");

    if (!sf_histint  (in,"n3",&ns)) sf_error("No n3= in input");
    if (!sf_histfloat(in,"d3",&ds)) sf_error("No d3= in input");
    if (!sf_histfloat(in,"o3",&os)) sf_error("No o3= in input");
    
    if (!sf_getbool("positive",&sign)) sign=true;
    /* initial offset orientation */

    type = 0.5 + ds/dh;
    
    dy = dh;
    oy = sign ? os + oh: os - oh - type*((nh-1)/type)*dh;
    ny = ns*type + nh - 1;

    nh2 = (nh+type-1)/type;
    sf_putint  (out,"n2",nh2);
    sf_putfloat(out,"d2",type*dh);

    sf_putint  (out,"n3",ny);
    sf_putfloat(out,"d3",dy);
    sf_putfloat(out,"o3",oy);

    if (!sf_histint(in,"esize",&esize)) {
	esize=4;
    } else if (0>=esize) {
	sf_error("wrong esize=%d",esize);
    }
    nt *= esize;

    if (NULL != sf_getstring("mask")) {
	msk = sf_output("mask");
	sf_settype(msk,SF_INT);

	sf_putint(msk,"n1",nh2);
	sf_putint(msk,"n2",ny);

	mask = sf_intalloc(nh2);
    } else {
	msk = NULL;
	mask = NULL;
    }

    trace = sf_charalloc(nt);
    zero  = sf_charalloc(nt);
    memset(zero,0,nt);

    sf_fileflush(out,in);
    sf_setform( in,SF_NATIVE);
    sf_setform(out,SF_NATIVE);
    
    sf_unpipe(in,(long) ns*nh*nt);
    pos = sf_tell(in);

    for (iy=0; iy < ny; iy++) {
	if (NULL != msk) ih2=0;
	for (ih=iy%type; ih < nh+iy%type; ih += type) {
	    is = sign? (iy - ih)/type : (iy + ih)/type - (nh - 1)/type;

	    if (is >= 0 && is < ns && ih < nh) {
		sf_seek(in,pos+(is*nh+ih)*nt,SEEK_SET);
		sf_charread(trace,nt,in);
		sf_charwrite(trace,nt,out);
		if (NULL != msk) mask[ih2++] = 1;
	    } else {
		sf_charwrite(zero,nt,out);
		if (NULL != msk) mask[ih2++] = 0;
	    }
	}
	if (NULL != msk) sf_intwrite(mask,nh2,msk);
    }

    sf_close();
    exit(0);
}
