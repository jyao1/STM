Build STM image step:
0) Install Visual Studio 2015.
1) Unzip StmPkg zip to target dir, e.g. c:\StmCode
   The final directory layout is like below:
     c:\StmCode\StmPkg\Core
     c:\StmCode\StmPkg\EdkII
     ...
2) Open command window, goto target dir, e.g. c:\StmCode
3) Type "preparestm.bat" to prepare environment, this need run only once.
4) Type "buildstm.bat" to build STM image.
   1) If user want debug build, please use "buildstm.bat" or "buildstm.bat DEBUG".
   2) If user want release build, please use "buildstm.bat RELEASE".

Build STM tool step:
1) Same as above
2) Same as above
3) Same as above
4) Type "buildstmtool.bat" to build STM tool.

