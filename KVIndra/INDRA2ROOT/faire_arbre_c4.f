c$Id: faire_arbre_c4.f,v 1.2 2006/10/19 14:32:43 franklan Exp $                                                                   
c---------------------------------------------------------------------  
c     Ici se trouvent reunies les diverses routines personnelles a    | 
c     l'utilisateur. Celui trouvera donc la routine INI_HBOOK,        | 
c     appelee en tout debut de programme et servant a initialiser     | 
c     les fichiers type Hbook par exemple. La routine INI_RUN est     | 
c     appelee a chaque fois que commence la lecture d'un run DST.     | 
c     La routine TRAITEMENT est appelee a CHAQUE evenement et permet  | 
c     de realiser l'analyse des donnees recuperees via les COMMONs    | 
c     definis dans VEDA_6 INCL ( cf fichier VEDA_6 INCL ).            | 
c     La routine FIN_RUN est appelee a la fin de la lecture d'un      | 
c     run DST et la routine CLOSE_HB est appelee en toute fin de      | 
c     lecture.                                                        | 
c                                                                     | 
c                                                                     | 
c Version du 12/12/94                                                 | 
c --------------------------------------------------------------------  
c                                                                       
      subroutine INI_HBOOK                                              
c                                                                       
c --- Hmemory donne la taille memoire en octets utilise par HBOOK...    
c --- Sa valeur peut etre diminuee ou augmentee suivant le nombre       
c --- de Hbooks crees ...                                               
c                                                                       
      include 'veda_rel.incl'                                           
	character*1 campaign
                                                                        
	call getenvf('CAMPAGNE', campaign)
	                                                                 
c --- Fin de la routine                                                 
                                                                        
      return                                                            
      end                                                               
                                                                        
c---------------------------------------------------------------------  
c --- Routine appelee evt par evt, les caracteristiques de l'evt        
c --- sont dans veda_6 INCL                                             
c---------------------------------------------------------------------  
c --- Compteurs disponibles sur la lecture des cartouches disponibles   
c --- via VEDA_REL INCL :                                               
c                                                                       
c   - Nbloclus  : Nbre de blocs lus dans le fichier courant             
c   - Nbloc_tot : Nbre TOTAL de blocs lus (cumul de Nbloclus)           
c   - Nevtlus   : Nbre d'evenements lus dans le fichier courant         
c   - Nevt_tot  : Nbre total d'evenement lus (cumul de Nevtlus)         
c                                                                       
c---------------------------------------------------------------------  
c --- Variables logiques disponibles concernant la lecture :            
c                                                                       
c   - Fin_lect_run : valeur a .False. pour la lecture des evenements    
c                     du fichier courant et a .True. lorsqu'on ne       
c                     veut plus lire d'evt dans le run DST courant.     
c                                                                       
c   - Fin_lect_gen : valeur a .False. pour la lecture des evenements    
c                     de tous les fichiers courant et a .True. lors-    
c                     qu'on ne veut plus lire du tout d'evt .           
c                                                                       
c   Ces 2 variables sont a .False. lorsqu'on demarre la lecture et      
c   peuvent etre passees a .True. lorsqu'on desire ne plus lire         
c   d'evenements soit dans le run courant ( on reprend alors la         
c   lecture des evts au run suivant s'il existe) soit pour le           
c   job ( on arrete alors proprement le batch ).                        
c                                                                       
c---------------------------------------------------------------------  
      subroutine TRAITEMENT                                             
                                                                        
      include 'veda_rel.incl'                                           
      include 'veda_5.incl'                                             
      include 'veda_6.incl'                                             
c      include 'anin_1.incl'                                             
c      include 'anin_3.incl'                                             
c      include 'simingeo.incl'                                           
      common /rootwrite/ necrit, nfiles , nrustines, nrustines_camp   
		character*60 filename                               
      integer*4  indice(500)
      integer*4  imult,icans

c ---- Inserez ici le traitement evt par evt                            
c      call initevt()
      pi=3.1415927                                                      
      
      if(npart_traite.gt.0) then
      	necrit=necrit+1
      	write(88,*) num_evt_brut
			write(88,*) npart_traite
      	do j=1,npart_traite
        		write(88,*) a(j)
        		write(88,*) z(j)
		  		write(88,*) z_indra(j)
		  		write(88,*) a_indra(j)
		  		
				if (code(j).ge.2.and.code(j).le.11.and.
     1             ecode(j).ge.1.and.ecode(j).le.2) then
					if (z(j).eq.0) then
						code(j)=15
						nrustines=nrustines+1
					endif
					if ( ener(j).le.0 ) then
						ener(j)=0
						ecode(j)=14
						code(j)=15
						nrustines=nrustines+1
					endif
					if (z(j).ge.1.and.a(j).eq.0) then
						code(j)=15
						nrustines=nrustines+1
					endif
					if ( icou(j).le.0.or.icou(j).ge.18.or.
     &					imod(j).le.0.or.icou(j).ge.25 ) then
	  					code(j)=15
						nrustines=nrustines+1
	  				endif
				endif
				
        		write(88,*) ener(j)
		  		write(88,*) icou(j)
        		write(88,*) imod(j)
				write(88,*) code(j)
		  		write(88,*) ecode(j)
		  		write(88,*) de_mylar(j)
		  		write(88,*) de1(j)
		  		write(88,*) de2(j)
		  		write(88,*) de3(j)
		  		write(88,*) de4(j)
				write(88,*) de5(j)
				write(88,*) mt(j)
				
				do kk=1,4
	 				write(88,*) code_idf(kk,j)
      		enddo
      		
      		icans=0
				do kk=1,15
					if(canal(j,kk).ne.0) then
	 					icans=icans+1
      			endif
      		enddo
      		write(88,*) icans
				do kk=1,15
      			if(canal(j,kk).ne.0) then
						write(88,*) kk,canal(j,kk)
      			endif
				enddo
			enddo    
		
			if(mod(necrit, 10000).eq.0) then
				nfiles = nfiles+1
				close(88)
				if(nfiles.lt.10) then
					write(filename,'(a,i1,a)') 'arbre_root_', nfiles, '.txt'
      		else if (nfiles.lt.100) then
					write(filename,'(a,i2,a)') 'arbre_root_', nfiles, '.txt'
      		else if (nfiles.lt.1000) then
					write(filename,'(a,i3,a)') 'arbre_root_', nfiles, '.txt'
     			else 
					write(filename,'(a,i4,a)') 'arbre_root_', nfiles, '.txt'
				endif 																					  
      		print *,'Opening file for writing : ', filename
				open(unit=88,file=filename,status='new')
			endif
		                                                                        
		endif !if(imult > 0) then
                                                                        
                                                                        
      return                                                            
      end                                                               
                                                                        
c-------------------------------------------------------------------    
c --- Routine appelee en debut de lecture d'un run                      
c-------------------------------------------------------------------    
      subroutine INI_RUN()                                              
                                                                        
      include 'veda_rel.incl'                                           
      include 'veda_5.incl'                                             
      include 'veda_6.incl'                                             
      include 'veda_11.incl'   
c      include 'anin_1.incl'                                             
      common /rootwrite/ necrit, nfiles, nrustines, nrustines_camp
	  character*60 filename                                 
                                                                        
c --- Faites ici les initialisations de runs                            
      print *,'Debut du traitement du run ',numerun,'...'               
      	  
c --- open first text file for writing events
		filename='list_of_files'
		open(unit=89,file=filename,status='new')
		write(89,*) 'run_number=',numerun
	   
		nfiles = 1
		write(filename,'(a,i1,a)') 'arbre_root_', nfiles, '.txt'
		print *,'Opening file for writing : ', filename
      open(unit=88,file=filename,status='new')                                                                        
c --- write number of run at beginning of text file
c	  	write(88,*) numerun
	  
      necrit=0
      nrustines=0
      nrustines_camp=0
		
      xmnuc=931.5                                                       
      ptot=sqrt((esura+xmnuc)**2-xmnuc**2)*aproj                        
      ztot=zcib+zproj                                                   
      vproj=ptot/(aproj*(esura+xmnuc))                                  
      zvtot=zproj*vproj                                                 
      vcm=aproj*vproj/(aproj+acib)                                      
      ecm=aproj*acib*esura/(aproj+acib)**2  
      iap=aproj                            
      izp=zproj                            
      iac=acib                            
      izc=zcib                            
      print 10,esura,projectile,aproj,zproj,cible,acib,zcib             
      print 20,ptot,vproj,zvtot                                         
      print 30,vcm,ecm
      
		filename=nomjob(1:long_job)//'.info'
		open(unit=87,file=filename,status='new')
		write(87,*) numerun, zproj, aproj, zcib, acib, esura
		close(87)	

      return                                                            
10    format(1x,'E/A=',f10.2,/,1x,'Projectile :',a3,2(1x,f4.0),/,       
     1                         1x,'Cible      :',a3,2(1x,f4.0))         
20    format(1x,'Ptot  :',f15.3,'  Vproj/C :',f6.4,'    ZVproj :',      
     1                                             f15.3)               
30    format(1x,'Vcm/C :',f6.4,'    Ecm disp :',f10.4,' MeV/A.',/)      
40    format(2(1x,i3),3(1x,f10.2),' 0 0. 0 0 0 ',6(1x,'0.'),' 0')
      end                                                               
                                                                        
c-------------------------------------------------------------------    
c --- Routine appelee en fin de lecture d'un run                        
c-------------------------------------------------------------------    
      subroutine FIN_RUN()
                                                                        
      include 'veda_rel.incl'                                           
      include 'veda_5.incl'                                             
      include 'veda_6.incl'                                             
      common /ttime/ ttot,tmin,tmax                                     
      common /rootwrite/ necrit, nfiles, nrustines, nrustines_camp                             
                                                                        
		close(88)
      
		write(89,*) 'file_number=',nfiles
		write(89,*) 'read_evts=',nevt
		write(89,*) 'trigger_rustines=',nrustines 
		write(89,*) 'write_evts=',necrit
		write(89,*) 'trigger_rustines_camp=',nrustines_camp 
		  
		close(89)  
		
		print *,'Fin de traitement du run ',numerun,'...'                 
      print *,'Evenements traites       :',nevt,'/',necrit
      print *,'Taux d''acceptes           :',necrit*100./nevt                        
      print *,'Temps moyen de traitement:',ttot/float(nevt)             
      print *,'Temps min                :',tmin                         
      print *,'Temps max                :',tmax                         
      print *,'Number of files written :', nfiles                                                       
      print *,' '                                                       
c write total number of events at end of last file
	   
		return                                                            
      end                                                               

c-------------------------------------------------------------------    
c --- Routine appelee en fin de lecture d'une tache                     
c-------------------------------------------------------------------    
      subroutine close_hb                                               
                                                                        
      include 'veda_rel.incl'                                          
      include 'veda_5.incl'                                            
      include 'veda_6.incl'                                            

c --- Inserez ici les fermetures de fichiers (Hbooks, ... )           
                                                                        
                                                                             
      return                                                          
      end                                                              
