/*
 * This file is part of bsod-server
 *
 * Copyright (c) 2004 The University of Waikato, Hamilton, New Zealand.
 * Author: Jesse Pouw-Waas
 *          
 * All rights reserved.
 *
 * This code has been developed by the University of Waikato WAND 
 * research group. For further information please see http://www.wand.net.nz/
 *
 * bsod-server is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * bsod-server is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bsod-server; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id$
 *
 */


#include "Blacklist.h"

blacklist::blacklist(const char *path, int bl_cnt, int save_int)
{
	BLpath = path;		// Blacklist path
	BL_cnt = bl_cnt;	// # of concurent blacklists
	Save_int = save_int;	// Blacklist save interval (seconds)
	ptime = 0.0f;

	head = new BL;		// Blacklist head node
	cur = head;
	
	for(int i=0;;i++)	// Build blacklist loop
	{
		cur->InUse = false;
		
		for(int x=0;x<256;x++)
			for(int y=0;y<256;y++)
				cur->List[x][y] = false;

		if(i<(bl_cnt-1))
		{
			cur->next = new BL;
			cur = cur->next;
		}
		else
		{
			cur->next = head;
			cur = head;
			
			load();
			return;
		}
	}
}

blacklist::~blacklist()
{

}

bool blacklist::poke(libtrace_packet_t *packet)
{
	libtrace_ip *ip_h = trace_get_ip(packet);
	int8_t pak_dir = trace_get_direction(packet);
	uint8_t *mask = NULL;

	if(ptime==0.0f)
		ptime = trace_get_seconds(packet)+Save_int;
	
	save(trace_get_seconds(packet));
	
	if(!ip_h)
		return 0;
	
	if(pak_dir==1) 		//INCOMING
	{
		mask = (uint8_t*)(&(ip_h->ip_dst.s_addr));

		if(read(mask)==0)
		{
			//printf("\nIBR: %u.%u.%u.%u",mask[0],mask[1],mask[2],mask[3]);
			return 1;
		}
		else
		{
			return 0;
		}	
	}
	else if(pak_dir==0)	//OUTGOING
	{
		mask = (uint8_t*)(&(ip_h->ip_src.s_addr));

		write( mask );
		//if(write(mask)==1);
			//printf("\nBlacklisting: %i.%i.%i.%i",mask[0],mask[1],mask[2],mask[3]);
		
		return 0;
		
	}

	return 0;
}

bool blacklist::read(uint8_t *ip)
{
	BL *itr=head;
	std::string base;

	base += itoa(ip[0]);
	base += ".";
	base += itoa(ip[1]);

	for(int i=0;i<BL_cnt;i++)
	{
		if(itr->FName==base)
		{
			if(itr->List[ip[2]][ip[3]]==1)
			{
				return 1;
			}
			else
				return 0;
		}
		
		if(itr->InUse==0)
			return 0;

		itr=itr->next;
	}
	
	return 0;
}

bool blacklist::write(uint8_t *ip)
{	
	BL *itr=head;
	std::string base;

	base += itoa(ip[0]);
	base += ".";
	base += itoa(ip[1]);

	for(int i=0;i<BL_cnt;i++)
	{	
		if(itr->FName==base)
		{
			if(itr->List[ip[2]][ip[3]]==0)
			{
				itr->List[ip[2]][ip[3]]=1;
				return 1;
			}
			else
				return 0;
		}

		if(itr->InUse==0)
		{
			break;
		}
		
		itr=itr->next;
	}

	delBL(cur);

	std::string fpath = BLpath + base;

	cur->FName = base;
	cur->InUse = 1;
	cur->List[ip[2]][ip[3]]=1;
	cur = cur->next;

	//printf("n");
	return 1;
}

bool blacklist::load()
{
	std::string 	FNdata=BLpath,
		partPath;
	FNdata+="BL.dat";
	uint32_t x,y;
	char buffer[100];//,buf[100];

	FILE *loadBL = fopen(FNdata.c_str(),"r");
	FILE *Fhndl=NULL;
	
    if(loadBL)
    {	
	while(!feof(loadBL))
	{
		if(fscanf(loadBL,"%s\n",buffer))
		{
			cur->FName = buffer;
			partPath=BLpath+buffer;
			cur->InUse=1;
			Fhndl = fopen(partPath.c_str(),"r");
			printf("\n\nBlacklist: %s",partPath.c_str());
			
			while(Fhndl && !feof(Fhndl))
			{
				if(fscanf(Fhndl,"%u.%u\n",&x,&y))
				{
					//printf("\n%u.%u",x,y);
					cur->List[x][y]=1;
				}
				else
					break;
			}
			
			fclose(Fhndl);
			Fhndl=NULL;
		}
		cur=cur->next;
	}
	fclose(loadBL);
    }
	return( true );
}

bool blacklist::save(double ctime)
{
   if(ptime<ctime)
   {
	ptime=ctime+Save_int;
	
	std::string FNdata=BLpath;
	FNdata+="BL.dat";

	std::string blnam;

	FILE *BLdat = fopen(FNdata.c_str(),"w");
	FILE *Fhndl = NULL;
	
	if(BLdat)
	{
		BL *itr=head;
		
		for(int i=0;i<BL_cnt;i++)
		{
			if(itr->InUse)
			{
				fprintf(BLdat,"%s\n",itr->FName.c_str());

				blnam = BLpath+itr->FName.c_str();
				
				Fhndl = fopen(blnam.c_str(),"w");
					
				if(Fhndl)
				{
					for(int x=0;x<256;x++)
						for(int y=0;y<256;y++)
							if(itr->List[x][y]==1)
								fprintf(Fhndl,"%u.%u\n",(uint8_t)x,(uint8_t)y);
					fclose(Fhndl);
				}
			}
			else
				break;

			itr=itr->next;
		}

		fclose(BLdat);
	}
   }
   return( true );
}

void blacklist::delBL(BL *die)
{
	if(!(die->InUse))
		return;

	for(int x=0;x<256;x++)
		for(int y=0;y<256;y++)
			die->List[x][y] = false;
}

char* blacklist::itoa(int input)
{
    static char buffer[10];
    snprintf(buffer,sizeof(buffer),"%d",input);

    return buffer;
}
