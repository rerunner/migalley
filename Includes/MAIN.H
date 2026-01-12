//------------------------------------------------------------------------------
//Filename       main.h
//System         
//Author         Paul.   
//Date           Wed 5 Jul 1995
//Description    
//------------------------------------------------------------------------------
#ifndef	main_Included
#define	main_Included

#include	"GETSET.H"


class	Main : public	GetSet
{
	public:
//TempCode MGA 06Oct95 		void	Main::EmitSysErr(char *, ...);
		void	Main::MainInit(void);
		void	Main::QuitGame(void );
		void	Main::RowanMessage();
		void	Main::DoScreen(MigWindow *);						//JIM 12Dec95

 		Bool	Main::InitFileSystem(InitDisplayDef&);

	private:

	protected:

};

	extern Main _Main;		//For 'EmitSysErr'			//JIM 21Sep95

#endif
