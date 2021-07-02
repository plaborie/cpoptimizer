using CP;

/* 
 This model solves a school time tabling problem.
 Given teacher skills, room equipment and pupil course requirement,
 the model generates for each course a time table specifying :
    - a teacher
    - a start time
    - a room
  
constraints are used to:

   - ensure the course ends after it starts         
   - ensure course numerotation is chronological     
   - ensure that a teacher is required once at any time point.  
   - ensure the teacher can teach the discipline 
   - ensure that a room is required once at any time point.
   - ensure the room can support the discipline
   - ensure that a class follows one course at a time      
   - ensure that for given class and discipline, the teacher is always the same  
   - ensure a course starts and end the same halfday
   - insert break duration between specified disciplines
   - avoid to have the same discipline taught twice a day
   - ensure that the morning disciplines end in the morning


Note: To reduce the amount of decision variable, we choose to use
course start times as time points where uniqueness of resources (classes, 
teachers and rooms) is enforced.

This model is greater than the size allowed in trial mode. 
You therefore need a valid license to run this example with the 
large data set or you can use timetabling-small.dat. 
See the Licensing Scheme document for details. 
*/
  
// increase this value to build a larger model.
// this will multiply the number of instances of each course by the given value
int ModelScale = 10;

tuple Pair {
  string a;
  string b;
};
//$doc:_requirement
tuple Requirement {
   string Class;            // a set of pupils
   string discipline;       // what will be taught
   int    Duration;         // course duration
   int    repetition;       // how many time the course is repeated
};
//end:_requirement
//
// user given model data
//

{Pair} NeedBreak = ...;                   // disciplines that should not be contiguous in time
{string} MorningDiscipline = ...;         // disciplines that must be taught in the morning
{Pair} TeacherDisciplineSet = ...;        // what are the teacher skills
{Pair} DedicatedRoomSet = ...;            // a set of disciplines requiring special rooms
{Requirement} RequirementSet = ...;       // the educational program
{string} Room = ...;                      // the set of available rooms
int BreakDuration = ...;                  // time interval between two disciplines
int DayDuration = ...;                    // must be even (morning duration equals afternoon duration)
int NumberOfDaysPerPeriod = ...;          // how many worked days per period
//
// vocabularies
//

{string} Class = {c | <c,d,u,n> in RequirementSet };
{string} Teacher = { t | <t,d> in TeacherDisciplineSet };
{string} Discipline =  {d | <t,d> in TeacherDisciplineSet };


int MaxRoom = card(Room);
int MaxTeacher = card(Teacher);

// convenience expressions for room compatibility
//
//$doc:_possibleRoom_booltable
int PossibleRoom[d in Discipline, x in Room] = 
  <x,d> in DedicatedRoomSet 
  || 0 == card({<z,k> | z in Room, k in Discipline
               : (<x,k> in DedicatedRoomSet) 
                 || (<z,d> in DedicatedRoomSet)});
//end:_possibleRoom_booltable  

//
// convenience expressions for teacher skills
//

// possible teacher disciplines
//$doc:_possibleTeacherDisc
{string} PossibleTeacherDiscipline[x in Teacher] = {d | <x,d> in TeacherDisciplineSet };
//end:_possibleTeacherDisc



//
// convenience expressions for requirement instantiation
//

// for a given requirement, an instance is one course occurrence
//$doc:_courseinstance
tuple Instance {
  string Class;
  string discipline;
  int    Duration;
  int    repetition;
  int    id;
  int    requirementId;
};
//end:_courseinstance
//$doc:_instgeneration

{Instance} InstanceSet = { 
  <c,d,t,r,i,z> | <c,d,t,r> in RequirementSet
                , z in ord(RequirementSet,<c,d,t,r>) .. ord(RequirementSet,<c,d,t,r>)
                , i in 1..r*ModelScale
};

range CourseId = 0..card(InstanceSet)-1;


//end:_instgeneration

//
// time expressions
//
//$doc:_timerange
int HalfDayDuration = DayDuration div 2;
int MaxTime = (((sum(i in InstanceSet) i.Duration) + DayDuration) div DayDuration)*DayDuration;
range Time = 0..MaxTime-1;
//end:_timerange
//
//

// decision variables
//
//$doc:_dvarstartroomteacher

int CourseIndex[i in InstanceSet] = ord(InstanceSet, i);

tuple CT {
  int cid;
  string teacher;
}

{CT} CourseTeacher = { <c,t> | i in InstanceSet, t in Teacher,  c in CourseId : c == CourseIndex[i] && i.discipline in PossibleTeacherDiscipline[t] };

tuple CR {
  int cid;
  string room;
  };
  
{CR} CourseRoom = { <c,r> | i in InstanceSet, r in Room,  c in CourseId : c == CourseIndex[i] && 1 == PossibleRoom[i.discipline, r] };

tuple triplet {
  int id1;
  int id2;
  int value;
};

{triplet} Breaks = { <ord(Discipline,a),ord(Discipline,b),BreakDuration> 
  |  <a,b> in NeedBreak : a != b
} union { <ord(Discipline,a),ord(Discipline,a),DayDuration>
  | a in Discipline
};
  
dvar interval courses[i in InstanceSet] in Time size i.Duration;
dvar sequence classCourses[c in Class] 
  in all(i in InstanceSet : i.Class == c) courses[i]
  types all(i in InstanceSet : i.Class == c) ord(Discipline, i.discipline);
dvar interval courseRoom[cr in CourseRoom] optional in Time;
dvar interval courseTeacher[ct in CourseTeacher] optional in Time;

dexpr int room[i in InstanceSet] = sum(cr in CourseRoom : cr.cid == CourseIndex[i])
  presenceOf(courseRoom[cr])*ord(Room, cr.room);
dexpr int teacher[i in InstanceSet] = sum(ct in CourseTeacher : ct.cid == CourseIndex[i])
  presenceOf(courseTeacher[ct])*ord(Teacher, ct.teacher);
dvar int classTeacher[c in Class][d in Discipline];
    
//end:_dvarstartroomteacher


execute {
   writeln("MaxTime = ", MaxTime);
   writeln("DayDuration = ", DayDuration);
   writeln("Teacher = ", Teacher);
   writeln("Discipline = ", Discipline);
   writeln("Class = ", Class);
   writeln("Breaks = ", Breaks);
   cp.param.BranchLimit= 20000;
//$doc:_search
}

dexpr int makespan = max(i in InstanceSet) endOf(courses[i]);
// minimize makespan
minimize makespan;

subject to { 
  forall(i in InstanceSet) {
    alternative(courses[i], all(ct in CourseTeacher : ct.cid == CourseIndex[i]) courseTeacher[ct]);
    alternative(courses[i], all(cr in CourseRoom : cr.cid == CourseIndex[i]) courseRoom[cr]);
  }
  forall(t in Teacher)
    noOverlap(all(ct in CourseTeacher : ct.teacher == t) courseTeacher[ct]);
  forall(r in Room)
    noOverlap(all(cr in CourseRoom : cr.room == r) courseRoom[cr]);
  forall(c in Class)
    noOverlap(classCourses[c], Breaks);
    

  // makespan >= max(c in Class) sum(r in InstanceSet : r.Class == c) r.Duration;
  
  forall(i in  InstanceSet)
     classTeacher[i.Class,i.discipline] == teacher[i];
   

  // ensure a course starts and end the same halfday
  //$doc:_halfday_ct
  forall(i in InstanceSet : i.Duration > 1)
    (startOf(courses[i]) div HalfDayDuration) == ((endOf(courses[i])-1) div HalfDayDuration);
    
//end:_needbreak_ct     
  // avoid to have the same discipline taught twice a day
//$doc:_onedisc_aday_ct
  forall(ordered i,j in InstanceSet: i.discipline == j.discipline && i.Class == j.Class) 
    (startOf(courses[i]) div DayDuration) != (startOf(courses[j]) div DayDuration);
//end:_onedisc_aday_ct    
  // ensure that the morning disciplines end in the morning
//$doc:_morningonly_ct
  forall(d in MorningDiscipline, i in InstanceSet : i.discipline == d) 
    (startOf(courses[i]) % DayDuration) < HalfDayDuration;

};

//
// generate time table
//
tuple Course {
   string teacher;
   string discipline;
   string room;
   int    id;
   int    repetition;
};

//$doc:_course
{Course} timetable[t in Time][c in Class] = {
  <p,d,r,i,n> 
  | d in Discipline
  , r in Room
  , x in InstanceSet
  , n in x.repetition..x.repetition
  , p in Teacher 
  , i in x.id..x.id
  : (t >= startOf(courses[x]))
  && (t < endOf(courses[x]))
  && (x.Class == c)
  && (room[x] == ord(Room,r))
  && (ord(Teacher,p) == teacher[x])
  && (d == x.discipline) 
};
//end:_course  
   
// force execution of postprocessing expressions
//$doc:_postprocess
execute POST_PROCESS {
  timetable;
  courses;
  courseTeacher;
  courseRoom;
  teacher;
  room;
  writeln("InstanceSet = ", InstanceSet);
  writeln("CourseTeacher = ", CourseTeacher);
  writeln("CourseRoom = ", CourseRoom);
  writeln("MaxRoom = ", MaxRoom);
  writeln("MaxTeacher = ", MaxTeacher);
  writeln("courses = ", courses);
  writeln("courseTeacher = ", courseTeacher);
  writeln("courseRoom = ", courseRoom); 
  writeln("teacher = ", teacher);
  writeln("room = ", room);
  for(var c in Class) {
    writeln("Class ", c);
    var day = 0;
    for(var t = 0; t < makespan; t++) {
      if(t % DayDuration == 0) {
        day++;
        writeln("Day ", day);
      }
      if(t % DayDuration == HalfDayDuration) 
        writeln("Lunch break");
      var activity = 0;
      for(var x in timetable[t][c]) {
        activity++;
        writeln((t % DayDuration)+1, "\t",
                x.room, "\t", 
                x.discipline, "\t", 
                x.id, "/", 
                x.repetition, "\t", 
                x.teacher);
      }
      if(activity == 0)
        writeln((t % DayDuration)+1, "\tFree time");
    }
  }
}
//end:_postprocess
 
 