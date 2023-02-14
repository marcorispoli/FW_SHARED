<center><font size =6">Z190 SAME51 Shared Firmware Documentation</font></center>
<center><font size =5">Document Index</font></center>
<center><font size =4">REV : 1.0</font></center>


 <br/><br/>
 <br/><br/>
 <br/><br/>
 <br/><br/>

<center>
<font size =4">

|Date |Revision | Change log| Author|
|:---| :----: | :----: |:---|
|14/02/2023|1.0|First release| M. Rispoli|

</font>
</center>

<div style="page-break-after: always;"></div>

# Overview

This repository provides a set of library modules for 
the SAME51 based device in the Z190 project.

Every shared module shall provides a readme.md file saved into the sub directory of the module: this file  shall describe the usage of the module. The readme.md file is pointed by this document in the *Shared Module Documentation Link*.

The source of the shared module shall be documented with the Doxygen syntax:
+ the target application using the module will be responsible to include 
the doxygen modules into its documentation structure.

# Shared Module Documentation Links

+ [CAN Module Documentation][CANP]: this is the module implementing the Device Can Protocol;

[CANP]:./CAN/readme.md


