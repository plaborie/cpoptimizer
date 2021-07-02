
 /*********************************************
 * Internal CPO Training
 *********************************************/

 /*
  * Timetabling:
  *
  * 1. There are classes (groups of children), teachers, rooms, subjects
  * 2. Each teacher can teach a subset of the subjects.
  * 3. Each room can host a subject of the subjects.
  * 4. The composition of subjects is given for each class (all classes are equivalent)
  * 5. No class has the same subject twice in one day.
  * 6. There are 4 periods per week, 5 days per week.
  *
  * The goal is to produce a weekly timetable for all classes.
  */
  
using CP;
  
tuple SubjectInfo {
  string name;
  string mneumonic;
  int times;
};

int NumSubjects = ...;
range Subject = 0..NumSubjects - 1;
SubjectInfo subjectData[Subject] = ...;
  
int NumTeachers = ...;
range Teacher = 0..NumTeachers-1;

{string} canTeach[Teacher] = ...;

int NumClasses = ...;
range Class = 0..NumClasses-1; 
                     
int NumRooms = ...;
range Room = 0..NumRooms-1;
  
{string} canHost[Room] = ...;

int NumDays = ...;
