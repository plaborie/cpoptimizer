/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * School timetabling
 ****************************************************/

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
int NumSlots = sum (s in Subject) subjectData[s].times;
int NumPeriods = NumSlots div NumDays;

range Day = 0..NumDays - 1;
range Period = 0..NumPeriods - 1;
range Slot = 0..NumSlots - 1;

////////////////////////////////////////////////////////////////////

tuple TeacherSubjectCompat {
  int teach;
  int subj;
};

{TeacherSubjectCompat} tscompat = {
  <t,s> | t in Teacher, s in Subject : subjectData[s].mneumonic in canTeach[t]
};

tuple RoomSubjectCompat {
  int teach;
  int subj;
};

{RoomSubjectCompat} rscompat = {
  <r,s> | r in Room, s in Subject : subjectData[s].mneumonic in canHost[r]
};

dvar int where[Class][Slot] in Room;
dvar int teach[Class][Slot] in Teacher;
dvar int subj [Class][Slot] in Subject;

execute {
  cp.param.DefaultInferenceLevel = "Extended";
}

constraints {
  // In any slot, each class must be in different
  // rooms with different teachers
  forall (s in Slot) {
    allDifferent(all (c in Class) teach[c][s]);
    allDifferent(all (c in Class) where[c][s]);  
  }
  
  // Compatibility between room and subject, teacher and subject
  forall (s in Slot, c in Class) {
    allowedAssignments(tscompat, teach[c][s], subj[c][s]);
    allowedAssignments(rscompat, where[c][s], subj[c][s]);
  }
  
  // No class has the same subject twice per day
  forall (c in Class, d in Day)
    allDifferent(all (p in Period) subj[c][d * NumPeriods + p]);
        
  // Respect teaching composition for each class
  forall (c in Class, s in Subject)
    count(all (slot in Slot) subj[c][slot], s) == subjectData[s].times;
  
  // Symmetry between classes
  //
  forall (c1, c2 in Class : c2 == c1 + 1)
     where[c1][0] < where[c2][0];
} 

execute {
  for (var c = 0; c < NumClasses; c++) {
    writeln("Class " + c);
    for (var d = 0; d < NumDays; d++) {
      write("  Day " + (d + 1) + ": ");
      for (var s = 0; s < NumPeriods; s++) {
        write(subjectData[subj[c][d*4+s]].mneumonic + "(");
        write(where[c][d*NumPeriods+s] + " / " + teach[c][d*NumPeriods+s] + ")	");
      }
      writeln();
    }
  } 
}