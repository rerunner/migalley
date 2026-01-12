//------------------------------------------------------------------------------
//Filename       error.h
//System         
//Author         Martin Alderton
//Date           Fri 6 Oct 1995
//Description    
//------------------------------------------------------------------------------
#ifndef	error_Included
#define	error_Included

class	 Error  
{	//can't allow this to be half commented out!
	FILE	*logfile;										//JIM 01Aug96

	public:
		const char* exitmsg;
		Error(void );
		~Error(void );

		Error& ExitMode();
		Error& Say(const char *, ...);
		Error& SayAndQuit(const char *, ...);
		void Quit();
#ifdef __WATCOMC__
#pragma	aux Quit			aborts;
#pragma	aux SayAndQuit		aborts;
#endif
		#if	DEBUGGING
			void OpenLog(void );
			void WriteToLog(char *,...);
			void CloseLog(void );
			void KeyTrap(void );								//pd 01Nov95
			static	void	dummyproc(char*)	{;}
			static	void	dummyproc(char*,int)	{;}
			static	void	dummyproc(char*,int,int)	{;}
			static	void	dummyproc(char*,int,int,const char*)	{;}
			static	void	dummyproc(char*,int,int,int)	{;}
			static	void	dummyproc(char*,int,int,int,int)	{;}
			static	void	dummyproc(char*,int,int,int,const char*,int)	{;}
			static	void	dummyproc(char*,char*)	{;}
			static	void	dummyproc(char*,char*,char*)	{;}
			static	void	dummyproc(char*,char*,int,int)	{;}
			static	void	dummyproc(char*,char*,int)	{;}
			static	void	dummyproc(char*,int,char*)	{;}
		#endif
			static	void	ReportLastRunError();
			static void	PleaseReport(bool x)		{pleasereport=x;}
			static void	PleaseLog(bool x)			{pleaselog=x;}
			static void TerminateCondition(char* x=NULL)	
			{	if (x)
				{
					stackedterminatecondition=terminatecondition;
					terminatecondition=x;
				}
				else
				{
					terminatecondition=stackedterminatecondition;
					stackedterminatecondition=NULL;
				}
			}
	private:
		static const char*	terminatecondition;
		static const char*	stackedterminatecondition;
		static bool	pleasereport;
		static bool	pleaselog;
		static void terminate(); 
	protected:

};
extern	Error	_Error;


#ifdef NDEBUG
 #define EmitSysErr ExitMode().SayAndQuit(__FILE__ "%i ",__LINE__),Error::dummyproc
#else
 #define EmitSysErr ExitMode().Say(__FILE__ ": ").SayAndQuit
#endif
 #define ReallyEmitSysErr ExitMode().Say(__FILE__ ": ").SayAndQuit

#include <assert.h> //RERUN

#endif
