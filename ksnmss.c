#include "ksnmss.h"
#define infy 999999


void key_generation(u8 *system_seed, u8 *system_iv){
	u32	id_t,id;
	int j;
	u8 	idu8[seedlen]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
	sk_node sk[t];
	node	pk[t];
	node	leaf;
	sk_node	temp;
	int xbyte[16][4];
	mssnode treestack[h+1];
	mssnode nodestack;
	int 	top=-1;
	u8	rdp[sklen];
	ECRYPT_ctx seed_ctx;
	ECRYPT_keysetup(&seed_ctx,system_seed,256,64);
	ECRYPT_ivsetup(&seed_ctx,system_iv);

	for(id=0;id<usr;id++){
		id_t=id;
		idu8[0] = (u8)(id_t & 0xFF);	id_t = id_t >> 8;
		idu8[1] = (u8)(id_t & 0xFF);	id_t = id_t >> 8;
		idu8[2] = (u8)(id_t & 0xFF);	id_t = id_t >> 8;
		idu8[3] = (u8)(id_t & 0xFF);
		ECRYPT_keysetup(&seed_ctx,system_seed,256,64);
		ECRYPT_ivsetup(&seed_ctx,system_iv);
		generate_secret_key_OTS(&seed_ctx, idu8, sk);
		set_Key(id,0);
		generate_public_key_OTS(sk, pk, A);
		leaf = create_L_tree(pk,id);
		memcpy(nodestack.key, leaf.key,pklen);
		nodestack.height = 0;
		nodestack.indx = id;

		while((top!=-1) && (treestack[top].height==nodestack.height)){
			if (nodestack.indx == 1)
				memcpy(auth[nodestack.height].key, nodestack.key,pklen);

			if (nodestack.indx == 3){
				if(nodestack.height < (h-2)){
					memcpy((instance[nodestack.height].v).key, nodestack.key,pklen);
					instance[nodestack.height].finalized = 1;
					instance[nodestack.height].lowheight = infy;
					instance[nodestack.height].top = -1;
				}else if (nodestack.height == (h-2)){
					memcpy(retain.key, nodestack.key,pklen);
				}
			}
			set_random_pad(nodestack.indx/2,nodestack.height+l+1,rdp);
			memcpy(temp.key, treestack[top].key,pklen);
			for(j=0;j<merlen;j++) 
				temp.key[pklen-merlen+j] = temp.key[pklen-merlen+j] ^ nodestack.key[j];
			memcpy(temp.key+pklen, nodestack.key+merlen,pklen-merlen);
			for(j=0;j<sklen;j++) temp.key[j] = temp.key[j] ^ rdp[j];
			parse(xbyte, temp.key);
			set_Key(nodestack.indx/2,nodestack.height+l+1);
			SWIFFT(xbyte,A,nodestack.key);
			nodestack.height++;
			nodestack.indx = nodestack.indx/2;
			top --;

			
		}
		top++;
		treestack[top] = nodestack;
	}	

	memcpy(MSSPK.key,treestack[top].key,pklen);	
	
		
}

void generate_secret_key_OTS(ECRYPT_ctx *seed_ctx, u8 *idu8, sk_node *sk){
	int i;
	
	u8 key[sklen];
	u8 OTS_seed[seedlen];
	u8 component_seed[seedlen];
	u8 component_id[sklen]={1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 
				0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
                   		0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 
				0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
	ECRYPT_ctx OTS_seed_ctx, component_seed_ctx;
	

	ECRYPT_encrypt_bytes(seed_ctx,idu8,OTS_seed,seedlen);
	ECRYPT_keysetup(&OTS_seed_ctx,OTS_seed,256,64);
	ECRYPT_ivsetup(&OTS_seed_ctx,OTS_seed);

	for(i=0;i<255;i++){
		ECRYPT_encrypt_bytes(&OTS_seed_ctx,component_id,(sk+i)->key,sklen);
		component_id[0]=component_id[0]+1;
	}

	component_id[0]=0;
	component_id[1]=1;
	for(i=255;i<t;i++){
		ECRYPT_encrypt_bytes(&OTS_seed_ctx,component_id,(sk+i)->key,sklen);
		component_id[0]=component_id[0]+1;
	}
	
}


void generate_public_key_OTS(sk_node *sk, node *pk, vec A[16][4]){
	int i;
	int xbyte[16][4];
	for(i=0;i<t;i++){
		parse(xbyte, (sk+i)->key);
		SWIFFT(xbyte,A,(pk+i)->key);
	}
}

node create_L_tree(node *pk, u32 id){
	node 	internal_node[t/2];
	sk_node	temp;
	int i,j,k,w,l1;
	u32 id_t=id<<l;
	int xbyte[16][4];
	u8 rdp[sklen];

	for(i=0;i<6;i++){
		set_random_pad(id_t+i,0,rdp);
		memcpy(temp.key, (pk+2*i)->key,pklen);
		for(j=0;j<merlen;j++) 
			temp.key[pklen-merlen+j] = temp.key[pklen-merlen+j] ^ (pk+2*i+1)->key[j];
		memcpy(temp.key+pklen, (pk+2*i+1)->key+merlen,pklen-merlen);
		
		for(j=0;j<sklen;j++) temp.key[j] = temp.key[j] ^ rdp[j];
		parse(xbyte, temp.key);
		
		set_Key(id_t+i,0);
		SWIFFT(xbyte,A,(internal_node+i)->key);
	}

	id_t = id_t/2;
	for(i=0;i<3;i++){
		set_random_pad(id_t+i,1,rdp);
		memcpy(temp.key, internal_node[2*i].key,pklen);
		for(j=0;j<merlen;j++) 
			temp.key[pklen-merlen+j] = temp.key[pklen-merlen+j] ^ internal_node[2*i+1].key[j];
		memcpy(temp.key+pklen, internal_node[2*i+1].key+merlen,pklen-merlen);
		for(j=0;j<sklen;j++) temp.key[j] = temp.key[j] ^ rdp[j];
		parse(xbyte, temp.key);
		set_Key(id_t+i,1);
		SWIFFT(xbyte,A,(internal_node+i)->key);
	}

	k=3;
	for(i=6;i<131;i++){
		set_random_pad(id_t+i,1,rdp);
		memcpy(temp.key, (pk+2*i)->key,pklen);
		for(j=0;j<merlen;j++) 
			temp.key[pklen-merlen+j] = temp.key[pklen-merlen+j] ^ (pk+2*i+1)->key[j];
		memcpy(temp.key+pklen, (pk+2*i+1)->key+merlen,pklen-merlen);
		for(j=0;j<sklen;j++) temp.key[j] = temp.key[j] ^ rdp[j];
		parse(xbyte, temp.key);
		set_Key(id_t+i,1);
		SWIFFT(xbyte,A,(internal_node+k)->key);
		k++;
	}

	w = 1 << (l-3);
	l1 = 2;
	for(k=h+l-2;k>h;k--){
		id_t = id_t/2;
		for(i=0;i<w;i++){
			set_random_pad(id_t+i,l1,rdp);
			memcpy(temp.key, internal_node[2*i].key,pklen);
			for(j=0;j<merlen;j++) 
				temp.key[pklen-merlen+j] = temp.key[pklen-merlen+j]^internal_node[2*i+1].key[j];
			memcpy(temp.key+pklen, internal_node[2*i+1].key+merlen,pklen-merlen);
			for(j=0;j<sklen;j++) temp.key[j] = temp.key[j] ^ rdp[j];
			parse(xbyte, temp.key);
			set_Key(id_t+i,l1);
			SWIFFT(xbyte,A,(internal_node+i)->key);
		}
		w = w>>1;
		l1++;
	}
	
	return internal_node[0];
}


void ksnmss_sign(u32 id, u8 *ms, ksnmss_sig *sig){
	int i,j,k,le;
	u32	id_t;
	u8 	idu8[seedlen]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
	sk_node sk[t];
	node	pk[t];
	u8 sksum[sklen*8];
	ECRYPT_ctx seed_ctx;
	rg_elm sum,temp_rg;
	u8 temp;
	int tempp;
	clock_t t2,t1=0;
	double time_taken;
	double cycles[1000],min;
	uint32 tau;
	uint32 maxh;
	uint32 eo;
	sk_node	temp2;
	int xbyte[16][4];
	u32 uindx;
	u8 rdp[sklen];

	
	ECRYPT_keysetup(&seed_ctx,system_seed,256,64);
	ECRYPT_ivsetup(&seed_ctx,system_iv);
	id_t=id;
	idu8[0] = (u8)(id_t & 0xFF);	id_t = id_t >> 8;
	idu8[1] = (u8)(id_t & 0xFF);	id_t = id_t >> 8;
	idu8[2] = (u8)(id_t & 0xFF);	id_t = id_t >> 8;
	idu8[3] = (u8)(id_t & 0xFF);
	generate_secret_key_OTS(&seed_ctx, idu8, sk);
	set_Key(id,0);
	generate_public_key_OTS(sk, pk, A);
	
	convert_u82u256(ms);
	cff();
	
	k=0;
	for(i=0;i<sklen;i++){
		temp=1;
		for(j=0;j<8;j++){
			sksum[k]=(u8)(((sk[component_key[0]].key[i])&temp)>>j);
			k++; temp=temp<<1;
		}
	}
	for(le=1;le<tb2;le++){
		k=0;
		for(i=0;i<sklen;i++){
			temp=1;
			for(j=0;j<8;j++){
				sksum[k]=sksum[k]+(u8)(((sk[component_key[le]].key[i])&temp)>>j);
				k++; temp=temp<<1;
			}
		}
	}

	sig->id = id;
	memcpy(sig->message,ms,msglen);
	memcpy(sig->sksum,sksum,sklen*8);
	memcpy(sig->pk,pk,t*pklen);
	memcpy(sig->auth,auth,h*pklen);


	if(id < usr){
		//1
		eo = id&1;
		if(eo==0) tau = 0;
		else {	id_t = id+1;
			tau = 0;
			while((id_t&1)==0){
				tau++;
				id_t = id_t>>1;
			}
		}

		//2
		id_t = id>>(tau+1);
		if(((id_t&1)==0) && (tau<(h-1)))
			keep[tau]=auth[tau];

		//3
		if(eo==0)
			auth[0]=create_L_tree(pk,id);
	
		//4
		//if(eo==1){
		if(tau>0){
			id_t = id>>tau;
			set_random_pad(id_t,tau+l,rdp);
			memcpy(temp2.key, auth[tau-1].key,pklen);
			for(j=0;j<merlen;j++) 
				temp2.key[pklen-merlen+j] = temp2.key[pklen-merlen+j]^keep[tau-1].key[j];
				memcpy(temp2.key+pklen, keep[tau-1].key+merlen,pklen-merlen);
				for(j=0;j<sklen;j++) temp2.key[j] = temp2.key[j] ^ rdp[j];
				parse(xbyte, temp2.key);
				set_Key(id_t,tau+l);
				SWIFFT(xbyte,A,auth[tau].key);
			for(i=0;i<tau;i++){
				if(i<h-2){ memcpy(auth[i].key, (instance[i].v).key,pklen);
				}
				else if(i==(h-2)) memcpy(auth[i].key, retain.key,pklen);
			}
			for(i=0;i<tau;i++){
				tempp = id+1+3*(1<<i);
				if(tempp < usr){
					instance[i].finalized = 0;
					instance[i].startleaf = tempp;
					if(instance[i].top==-1)
						instance[i].lowheight = 0;
				}
			}

		}
		
		int minh,minindx,update=0;
		node 	leaf;
		mssnode	nodestack;
		for(i=0;i< updateiter;i++){
			minh=infy; //minindx=infy;
			for(j=0;j<h-2;j++){
				if((instance[j].lowheight!=infy) && (instance[j].finalized!=1)){
					if (minh>instance[j].lowheight){
						minh = instance[j].lowheight;
						minindx = j;
					}
				}
			}
			if(minh<infy){
				id_t=instance[minindx].startleaf;
				idu8[0] = (u8)(id_t & 0xFF);	id_t = id_t >> 8;
				idu8[1] = (u8)(id_t & 0xFF);	id_t = id_t >> 8;
				idu8[2] = (u8)(id_t & 0xFF);	id_t = id_t >> 8;
				idu8[3] = (u8)(id_t & 0xFF);
				ECRYPT_keysetup(&seed_ctx,system_seed,256,64);
				ECRYPT_ivsetup(&seed_ctx,system_iv);
				generate_secret_key_OTS(&seed_ctx, idu8, sk);
				set_Key(instance[minindx].startleaf,0);
				generate_public_key_OTS(sk, pk,A);
				leaf = create_L_tree(pk,instance[minindx].startleaf);

//set_Key(id_t,tau+l);

				if(minindx==0){
					memcpy((instance[minindx].v).key, leaf.key,pklen);
					instance[minindx].finalized = 1;
					instance[minindx].lowheight = infy;
				}
				else if(instance[minindx].top==-1){
					instance[minindx].top++;
					memcpy((instance[minindx].treestack[0]).key,leaf.key,pklen);
					(instance[minindx].treestack[0]).indx=instance[minindx].startleaf;
					(instance[minindx].treestack[0]).height=0;
					instance[minindx].startleaf = instance[minindx].startleaf+1;
				}
				else{
					memcpy(nodestack.key,leaf.key,pklen);
					nodestack.height = 0;
					nodestack.indx = instance[minindx].startleaf;
					while((instance[minindx].top!=-1) && (nodestack.height==(instance[minindx].treestack[instance[minindx].top]).height)){
						memcpy(temp2.key, (instance[minindx].treestack[instance[minindx].top]).key,pklen);
						for(k=0;k<merlen;k++) 
							temp2.key[pklen-merlen+k] = temp2.key[pklen-merlen+k]^nodestack.key[k];
						memcpy(temp2.key+pklen, nodestack.key+merlen,pklen-merlen);
						set_random_pad(nodestack.indx/2,nodestack.height+1+l,rdp);
						for(k=0;k<sklen;k++) temp2.key[k] = temp2.key[k] ^ rdp[k];
						parse(xbyte, temp2.key);
						set_Key(nodestack.indx/2,nodestack.height+1+l);
						SWIFFT(xbyte,A,nodestack.key);
						nodestack.height++;	
						instance[minindx].top--;
						nodestack.indx=nodestack.indx/2;
					}
					if(nodestack.height == minindx){
						memcpy((instance[minindx].v).key,nodestack.key,pklen);
						instance[minindx].finalized = 1;
						instance[minindx].lowheight = infy;
						instance[minindx].top = -1;
					}else{
						instance[minindx].top++;
						//memcpy((instance[minindx].treestack[instance[minindx].top]).key,nodestack.key,pklen);
						//(instance[minindx].treestack[instance[minindx].top]).height = nodestack.height;
						//(instance[minindx].treestack[instance[minindx].top]).indx = nodestack.indx;		
						(instance[minindx].treestack[instance[minindx].top]) = nodestack;
						instance[minindx].startleaf = instance[minindx].startleaf+1;
						
						if(instance[minindx].top==0)
							instance[minindx].lowheight = nodestack.height;
					}
				}
			}else break;
		}
	}
}



int ksnmss_verify(u32 id, u8 *ms, ksnmss_sig *sig){
	int 	i,j,k,le;
	u32	id_t;
	int 	x[16][64];
	u8 	idu8[seedlen];
	sk_node sk[t];
	node	pk[t];
	u32 	pksum[rglen];
	ECRYPT_ctx seed_ctx;
	rg_elm 	sum,temp;
	sk_node	temp_node;
	int xbyte[16][4];
	u32    verified;
	int tempp;
	u8 rdp[sklen];
	clock_t t2,t1=0;


	for(i=0;i<1024;i++){
		if(sig->sksum[i]>131) return 3;
	}
	convert_u82u256(ms);
	cff();

	for(i=0;i<16;i++){
		for(j=0;j<64;j++) x[i][j] = sig->sksum[i*64+j];
	}
	set_Key(id,0);
	gSWIFFT(x,A,pksum);

	convert_ring((sig->pk[component_key[0]]).key,&sum);

	for(le=1;le<tb2;le++){
		convert_ring((sig->pk[component_key[le]]).key,&temp);
		for(i=0;i<rglen;i++){
			sum.key[i] = sum.key[i]+temp.key[i];
		}
	}

	for(i=0;i<rglen;i++){
		sum.key[i] = sum.key[i]%257;
	}
	
	verified = 0;
	for(i=0;i<rglen;i++) verified = verified + (pksum[i]-sum.key[i]);
	//printf("%d\n",verified);
	if (verified != 0) return 2;
	
	node present,left,right;
	present= create_L_tree(sig->pk,id);

	for(i=0;i<h;i++){
		if(id%2==0){
			left = present; right=sig->auth[i];
		}
		else{
			left = sig->auth[i]; right=present;
		}
		memcpy(temp_node.key, left.key,pklen);
		for(j=0;j<merlen;j++) 
			temp_node.key[pklen-merlen+j] = temp_node.key[pklen-merlen+j] ^ right.key[j];
		memcpy(temp_node.key+pklen, right.key+merlen,pklen-merlen);
		set_random_pad(id/2,i+l+1,rdp);	
		for(j=0;j<sklen;j++) temp_node.key[j] = temp_node.key[j] ^ rdp[j];
		parse(xbyte, temp_node.key);
		set_Key(id/2,i+l+1);
		SWIFFT(xbyte,A,present.key);
		id = id/2;
	}

	verified = 0;
	for(i=0;i<pklen;i++){
		//verified = verified + (MSS[mmsize-1].key[i]-present.key[i]);
		verified = verified + (MSSPK.key[i]-present.key[i]);
	}
	
	if (verified == 0) return 0;
	else return 1;

}

void convert_ring(u8 *rg8, rg_elm *a){
	int i;
	for(i=0;i<8;i++){
		a->key[8*i+0] = rg8[9*i+0];
		a->key[8*i+0] = a->key[8*i+0] | ((u32)(rg8[9*i+1]&0x1)<<8);
		
		a->key[8*i+1] = ((u32)(rg8[9*i+1])>>1);
		a->key[8*i+1] = a->key[8*i+1]|((u32)(rg8[9*i+2]&0x3)<<7);

		a->key[8*i+2] = ((u32)(rg8[9*i+2])>>2);
		a->key[8*i+2] = a->key[8*i+2]|((u32)(rg8[9*i+3]&0x7)<<6);

		a->key[8*i+3] = ((u32)(rg8[9*i+3])>>3);
		a->key[8*i+3] = a->key[8*i+3]|((u32)(rg8[9*i+4]&0xF)<<5);

		a->key[8*i+4] = ((u32)(rg8[9*i+4])>>4);
		a->key[8*i+4] = a->key[8*i+4]|((u32)(rg8[9*i+5]&0x1F)<<4);

		a->key[8*i+5] = ((u32)(rg8[9*i+5])>>5);
		a->key[8*i+5] = a->key[8*i+5]|((u32)(rg8[9*i+6]&0x3F)<<3);

		a->key[8*i+6] = ((u32)(rg8[9*i+6])>>6);
		a->key[8*i+6] = a->key[8*i+6]|((u32)(rg8[9*i+7]&0x7F)<<2);

		a->key[8*i+7] = ((u32)(rg8[9*i+7])>>7);
		a->key[8*i+7] = a->key[8*i+7]|((u32)rg8[9*i+8]<<1);
	}
}

void pn(node *r){
	printf("\n");
	for(int i=0;i<pklen;i++)
		printf("%hhu ",r->key[i]);
	printf("\n");
}

void set_random_pad(u32 indx, u32 height, u8 randpad[sklen]){
	ECRYPT_ctx seed_ctx;
	ECRYPT_keysetup(&seed_ctx,randompad_seed,256,64);
	ECRYPT_ivsetup(&seed_ctx,randompad_iv);
	u8 ina[sklen]={0};


	ina[0] = indx & 255; indx = indx >>8;
	ina[1] = indx & 255; indx = indx >>8;
	ina[2] = indx & 255; indx = indx >>8;
	ina[3] = indx & 255;

	ina[64] = height;

	ECRYPT_keysetup(&seed_ctx,hk_seed,256,64);
	ECRYPT_ivsetup(&seed_ctx,hk_iv);
	ECRYPT_encrypt_bytes(&seed_ctx,ina,randpad,128);
	
}

