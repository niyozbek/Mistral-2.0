
/*
  Mistral 2.0 is a constraint satisfaction and optimisation library
  Copyright (C) 2009  Emmanuel Hebrard
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  The author can be contacted electronically at emmanuel.hebrard@gmail.com.
*/

#include <sstream>
#include <fstream>
#include <signal.h>
#include <assert.h>

#include <mistral_sat.hpp>
//#include <mistral_search.hpp>
#include <mistral_solver.hpp>
#include <mistral_variable.hpp>
#include <mistral_constraint.hpp>


//#define _DEBUG_RESTORE true
//#define _DEBUG_REWRITE true

Mistral::Solver* active_solver;
static void Mistral_SIGINT_handler(int signum) {
  std::cout << std::endl 
	    << " c *************************************** INTERRUPTED ***************************************" 
	    << std::endl;
  std::cout << active_solver->statistics << std::endl
	    << " c *************************************** INTERRUPTED ***************************************" 
	    << std::endl;
  exit(1);
}





// #ifdef _DEBUG_PROPAG 
// std::ostringstream *o_propag = NULL;
// #endif

Mistral::Solution::Solution( Vector< Variable >& vars ) {
  min_id = INFTY;
  max_id = -INFTY;
  int aux;
  for(unsigned int i=0; i<vars.size; ++i) {
    variables.add(vars[i].get_var());
    aux = variables[i].id();
    if(aux<min_id) min_id = aux;
    if(aux>max_id) max_id = aux;
  }

  values = new int[max_id-min_id+1];
  values -= min_id;

  for(unsigned int i=0; i<variables.size; ++i) {
    values[variables[i].id()] = variables[i].get_solution_int_value();
  }
}
Mistral::Solution::~Solution() {

#ifdef _DEBUG_MEMORY
  std::cout << "c delete solution" << std::endl;
#endif

  values += min_id;
  delete [] values;
}

std::ostream& Mistral::Solution::display(std::ostream& os) const {
  if(!variables.empty()) {
    os << variables[0] << ":" << values[variables[0].id()];
    for(unsigned int i=1; i<variables.size; ++i) {
      os << " " << variables[i] << ":" << values[variables[i].id()];
    }
  }
  return os;
}
   
int& Mistral::Solution::operator[](Variable x) {
  return values[x.id()];
}


Mistral::SolverParameters::SolverParameters() {
  initialise(); 
}
Mistral::SolverParameters::~SolverParameters() {
#ifdef _DEBUG_MEMORY
  std::cout << "c delete solver parameters" << std::endl;
#endif
}
void Mistral::SolverParameters::initialise() {
  find_all = 0;
  node_limit = 0;
  backtrack_limit = 0;
  propagation_limit = 0;
  fail_limit = 0;
  //restart_limit = 0;
  limit = 0;
  seed = 11041979;

  verbosity = 0;
  time_limit = -1;
  seed = 11041979;
  restart_policy = GEOMETRIC;
  restart_base = 256;
  restart_limit = 256;
  restart_factor = 1.33;
  activity_increment = 1e-2;
  normalize_activity = 0;
  init_activity = 1;
  forgetfulness = .75;
  randomization = 1; //2;
  shuffle = false; //true;
  activity_decay = 0.96;
  checked = 1;
  backjump = 0;
  value_selection = 2;
  dynamic_value = 0; //1;
}
Mistral::SolverParameters::SolverParameters(const SolverParameters& sp) {
  copy(sp);
}
void Mistral::SolverParameters::copy(const SolverParameters& sp) {
  propagation_limit = sp.propagation_limit;
  fail_limit = sp.fail_limit;
  seed = sp.seed;

  time_limit = sp.time_limit;
  restart_policy = sp.restart_policy;
  restart_base = sp.restart_base;
  restart_limit = sp.restart_limit;
  restart_factor = sp.restart_factor;
  activity_increment = sp.activity_increment;
  normalize_activity = sp.normalize_activity;
  init_activity = sp.init_activity;
  forgetfulness = sp.forgetfulness;
  randomization = sp.randomization;
  shuffle = sp.shuffle;
  activity_decay = sp.activity_decay;
  checked = sp.checked;
  backjump = sp.backjump;
  value_selection = sp.value_selection;
  dynamic_value = sp.dynamic_value;


  verbosity = sp.verbosity;
  find_all = sp.find_all;
  node_limit = sp.node_limit;
  backtrack_limit = sp.backtrack_limit;
  fail_limit = sp.fail_limit;
  restart_limit = sp.restart_limit;
  limit = sp.limit;
  time_limit = sp.time_limit;
}

Mistral::SolverStatistics::SolverStatistics() { 

  // VARNAME = {"virtual", "constant", "boolean", "range", "bitset", "list"};
  // METHOD_NAME = {
  //   "get_size"         ,
  //   "get_degree"       ,
  //   "get_min"          ,
  //   "get_max"          ,
  //   "get_initial_min"  ,
  //   "get_initial_max"  ,
  //   "get_min_pos"      ,
  //   "get_max_neg"      ,
  //   "next"             ,
  //   "prev"             ,
  //   "is_range"         ,
  //   "is_ground"        ,
  //   "equal"            ,
  //   "contain"          ,
  //   "intersect_range"  ,
  //   "included_range"   ,
  //   "includes_range"   ,
  //   "intersect_set"    ,
  //   "included_set"     ,
  //   "includes_set"     ,
  //   "intersect_to"     ,
  //   "union_to"         ,
  //   "remove"           ,
  //   "set_domain"       ,
  //   "set_min"          ,
  //   "set_max"          ,
  //   "set_domain_bitset",
  //   "remove_set"       ,
  //   "remove_interval"  ,
  //   "restore"          
  // };

initialise(); 


#ifdef _PROFILING
  init_prof();
#endif


}
Mistral::SolverStatistics::~SolverStatistics() {
#ifdef _DEBUG_MEMORY
  std::cout << "c delete solver statistics" << std::endl;
#endif
}
void Mistral::SolverStatistics::initialise() {
  objective_value = 0;
  num_variables = 0; 
  num_values = 0; 
  num_constraints = 0; 
  num_nodes = 0; 
  num_restarts = 0; 
  num_backtracks = 0;
  num_failures = 0; 
  num_propagations = 0;
  num_solutions = 0;
  num_filterings = 0;
  //start_time = 0.0;
  start_time = get_run_time();
  end_time = -1.0;

  outcome = UNKNOWN;

  base_avg_size = 0;
  learnt_avg_size = 0;
  literals = 0;
  small = 0;
}

#ifdef _PROFILING

void Mistral::SolverStatistics::init_prof() {
     total_propag_time = 0;
     total_branching_time = 0;
     total_restore_time = 0;

#ifdef _PROFILING_PRIMITIVE

     for(int i=0; i<NUM_METHODS; ++i) {
       for(int j=0; j<NUM_VARTYPES; ++j) {
	 prof_num[i][j] = 0;
     	 prof_time[i][j] = 0;
       }
     }

#endif
 
}


int **comparison_array;
int compar_prof(const void *a, const void *b)
{
  int xa = (*(int*)a)/NUM_VARTYPES;
  int ya = (*(int*)a)%NUM_VARTYPES;

  int xb = (*(int*)b)/NUM_VARTYPES;
  int yb = (*(int*)b)%NUM_VARTYPES;

  if (comparison_array[xa][ya]==comparison_array[xb][yb])
    return 0;
  else
    if (comparison_array[xa][ya] < comparison_array[xb][yb])
      return 1;
    else
      return -1;
}

#ifdef _PROFILING_PRIMITIVE

std::ostream& Mistral::SolverStatistics::print_profile(std::ostream& os) const {
  int N = NUM_METHODS*NUM_VARTYPES;
  int order[N];
  unsigned long long int total_calls = 0;
  double total_time = 0.0;

  int method;
  int vartype;

  //os << "\n" << N << std::endl;

  for(int i=0; i<N; ++i) {

    // method = i/NUM_VARTYPES;
    // vartype = i%NUM_VARTYPES;

    // os << method << " " << vartype << std::endl;

    // os << VAR_NAME[vartype] << "." << METHOD_NAME[method] << "(): " 
    //    << prof_num[method][vartype] << " calls ("
    //    << (double)(prof_num[method][vartype]*100) / (double)total_calls << "%), "
    //    << prof_time[method][vartype] << "s (" 
    //    << prof_time[method][vartype]*100 / (double)total_time << "%)" << std::endl;


    total_calls += prof_num[i/NUM_VARTYPES][i%NUM_VARTYPES];
    total_time += prof_time[i/NUM_VARTYPES][i%NUM_VARTYPES];
    order[i] = i;
  }

  comparison_array = new int*[NUM_METHODS];
  for(int i=0; i<NUM_METHODS; ++i) {
    comparison_array[i] = new int[NUM_VARTYPES];
    for(int j=0; j<NUM_VARTYPES; ++j)
      comparison_array[i][j] = prof_num[i][j];
  }

  //comparison_array = &((&(prof_time[0]))[0])

    //comparison_array = &((&prof_time[0])[0]);
  qsort(order, N, sizeof(int), compar_prof);

  for(int i=0; i<N; ++i) {
    method = order[i]/NUM_VARTYPES;
    vartype = order[i]%NUM_VARTYPES;

    if(prof_num[method][vartype] == 0) break;

    std::string vname = std::string(VAR_NAME[vartype]);
    std::string mname = std::string(METHOD_NAME[method]);

    

    //+"."+METHOD_NAME[method]+"()";

    os //<< VAR_NAME[vartype] << "." << METHOD_NAME[method] << "(): " 
      << std::left << std::setw(24) << vname+"."+mname+"()"
      << std::right << std::setw(10) << prof_num[method][vartype] << " calls ("
      << std::setprecision(6) << std::right << std::setw(7) 
      << (double)(prof_num[method][vartype]*100) / (double)total_calls << "%), "
      << std::right << std::setw(6) << prof_time[method][vartype] << "s (" 
      << std::setprecision(6) << std::right << std::setw(7) 
      << prof_time[method][vartype]*100 / (double)total_time << "%)" << std::endl;

  }

  for(int i=0; i<NUM_METHODS; ++i) {
    delete [] comparison_array[i];
  }
  delete [] comparison_array;

  return os;
}

#endif

#endif

std::ostream& Mistral::SolverStatistics::print_full(std::ostream& os) const {
  os << " c +" << std::setw(90) << std::setfill('=')
    //"=============================================================================
     << "+" << std::endl << std::setfill(' ')
     << std::left << std::setw(46) << " s  ";

  switch(outcome) {
  case SAT: 
    os << std::right << std::setw(47) << "SATISFIABLE" ;
    break;
  case OPT: 
    os << std::right << std::setw(47) << "OPTIMAL" ;
    break;
  case UNSAT: 
    os << std::right << std::setw(47) << "UNSATISFIABLE" ;
    break;
  case UNKNOWN: 
    os << std::right << std::setw(47) << "UNKNOWN" ;
    break;
  case LIMITOUT: 
    if(num_solutions > 0) 
      os << std::right << std::setw(47) << "SUBOPTIMAL" ;
    else 
      os << std::right << std::setw(47) << "LIMITOUT" ;
    //break;
  }
  os << std::endl
     << std::left << std::setw(46) << " v  0" << std::endl
     << std::left << std::setw(46) << " d  OBJECTIVE"
     << std::right << std::setw(46) << objective_value  << std::endl
     << std::left << std::setw(46) << " d  TIME"
     << std::right << std::setw(46) << (end_time - start_time)  << std::endl
     << std::left << std::setw(46) << " d  MEMORY"
     << std::right << std::setw(46) << (mem_used() / 1048576.0) << std::endl
     << std::left << std::setw(46) << " d  NODES"
     << std::right << std::setw(46) << num_nodes  << std::endl
     << std::left << std::setw(46) << " d  RESTARTS"
     << std::right << std::setw(46) << num_restarts << std::endl
     << std::left << std::setw(46) << " d  FAILURES"
     << std::right << std::setw(46) << num_failures << std::endl
     << std::left << std::setw(46) << " d  BACKTRACKS"
     << std::right << std::setw(46) << num_backtracks << std::endl
     << std::left << std::setw(46) << " d  PROPAGATIONS"
     << std::right << std::setw(46) << num_propagations << std::endl
     << std::left << std::setw(46) << " d  FILTERINGS"
     << std::right << std::setw(46) << num_filterings << std::endl
     << " c +" << std::setw(90) << std::setfill('=') << "+" << std::endl << std::setfill(' ');
  //<< " c +=============================================================================+" << std::endl;
  return os;
}
std::ostream& Mistral::SolverStatistics::print_short(std::ostream& os) const {
  os << " c |";
  os << std::right << std::setw(7) << num_variables << " |";
  os << std::right << std::setw(8) << num_values << " |";
  os << std::right << std::setw(7) << num_constraints << " |";
  os << std::right << std::setw(9) << num_nodes << " |" ;
  os << std::right << std::setw(11) << num_filterings << " |" ;
  os << std::right << std::setw(13) << num_propagations << " |" ;
  os << std::right << std::setw(9) << std::setprecision(5) ;
  os << (get_run_time() - start_time) << " |" ;
  os << std::right << std::setw(10) << objective_value ;
  os << " |";
  return os;
}
std::ostream& Mistral::SolverStatistics::display(std::ostream& os) const {
  if(end_time >= 0.0) print_full(os);
  else print_short(os);
  return os;
}
Mistral::SolverStatistics::SolverStatistics(const SolverStatistics& sp) {
  copy(sp);
}
void Mistral::SolverStatistics::copy(const SolverStatistics& sp) {
  objective_value = sp.objective_value;
  num_variables = sp.num_variables;
  num_constraints = sp.num_constraints; 
  num_values = sp.num_values; 
  num_nodes = sp.num_nodes; 
  num_restarts = sp.num_restarts; 
  num_backtracks = sp.num_backtracks;
  num_failures = sp.num_failures; 
  num_propagations = sp.num_propagations;
  num_solutions = sp.num_solutions;
  num_filterings = sp.num_filterings;
  start_time = sp.start_time;
  end_time = sp.end_time;
}
void Mistral::SolverStatistics::update(const SolverStatistics& sp) {
  objective_value = sp.objective_value;

  num_nodes += sp.num_nodes; 
  num_restarts += sp.num_restarts; 
  num_backtracks += sp.num_backtracks;
  num_failures += sp.num_failures; 
  num_propagations += sp.num_propagations;
  num_solutions += sp.num_solutions;
  num_filterings += sp.num_filterings;
  if(end_time < sp.end_time) end_time = sp.end_time;
}

Mistral::ConstraintQueue::ConstraintQueue()
{
  min_priority = 0;
  cardinality = -1;
  higher_priority  = -1;
  triggers = NULL;
  //taboo_constraint = NULL;
  _set_.initialise();
}



void Mistral::ConstraintQueue::reset_higher_priority() {
  while(--higher_priority>=min_priority && triggers[higher_priority].empty());
}

Constraint Mistral::ConstraintQueue::select(Mistral::Vector<Constraint>& constraints) {
  int cons_id = triggers[higher_priority].pop();
  Constraint cons = constraints[cons_id];
  _set_.fast_remove(cons_id);
  if(triggers[higher_priority].empty()) reset_higher_priority();
  //taboo_constraint = cons.freeze();
  return cons;
}
// inline void select(Constraint cons) {
//   //_set_.remove(cons->id);
//   taboo_constraint = cons.freeze();
// }

void Mistral::ConstraintQueue::clear() {
  while(higher_priority>=min_priority) triggers[higher_priority--].clear();
  _set_.clear();
  //taboo_constraint = NULL;
}

void Mistral::ConstraintQueue::declare(Constraint c, Solver *s) {
  
  // std::cout << "declare " << c << "(" << c.priority() << ") to the constraint queue" << std::endl;
  // std::cout << "was: [" << min_priority << ","
  //  	    << min_priority+cardinality-1 << "]" << std::endl;
  
  int cons_idx = c.id();
  int cons_priority = c.priority();
  
  int new_min_p = min_priority;
  int new_max_p = min_priority+cardinality-1;
  
  if(cons_idx == 0) solver = s;
  if(cons_priority < new_min_p || cons_priority > new_max_p) {
    if(cardinality > 0) {
      if(cons_priority < new_min_p) new_min_p = cons_priority;
      if(cons_priority > new_max_p) new_max_p = cons_priority;
    } else {
      new_min_p = cons_priority;
      new_max_p = cons_priority;
    }

    // std::cout << "now: [" << new_min_p << ","
    // 	    << new_max_p << "]" << std::endl;


    int new_cardinality = (new_max_p-new_min_p+1);
    Queue *aux_triggers = triggers;
    triggers = new Queue[new_cardinality];
    triggers -= new_min_p;
    for(int i=min_priority; i<min_priority+cardinality; ++i) {
      triggers[i] = aux_triggers[i];
      aux_triggers[i].cancel();
    }

    triggers[cons_priority].initialise(cons_idx, cons_idx+7);

    aux_triggers += min_priority;
    delete [] aux_triggers;

    min_priority = new_min_p;
    cardinality = new_cardinality;


    // std::cout << " ==> init " << triggers[cons_priority] << std::endl;


  } else {
    
    // std::cout << "no need to create a new trigger list" << std::endl;
    // std::cout << "extend " << triggers[cons_priority] << " with " << cons_idx << std::endl;
     
    if(!triggers[cons_priority].is_initialised()) {
      triggers[cons_priority].initialise(cons_idx, cons_idx+7);
    } else {
      triggers[cons_priority].extend(cons_idx);
    }
  }
  
  if(_set_.table)
    _set_.extend(cons_idx);
  else 
    _set_.initialise(cons_idx, cons_idx, BitSet::empt);

}

void Mistral::ConstraintQueue::initialise(Solver *s)
{
  solver = s;
  _set_.initialise(0,2*s->constraints.capacity,BitSet::empt);
  
  int min_p = INFTY;
  int max_p = -INFTY;
  for(unsigned int i=0; i<s->constraints.size; ++i) {
    if(s->constraints[i].priority() < min_p) min_p = s->constraints[i].priority();
    if(s->constraints[i].priority() > max_p) max_p = s->constraints[i].priority();
  }
  initialise(min_p, max_p, s->constraints.size);
}

void Mistral::ConstraintQueue::initialise(const int min_p, 
					  const int max_p,
					  const int size)
{
  cardinality = max_p-min_p+1;
  min_priority = min_p;
  triggers = new Queue[cardinality];
  for(int i=0; i<cardinality; ++i) {
    triggers[i].initialise(size);
  }
  triggers -= min_priority;
}

Mistral::ConstraintQueue::~ConstraintQueue() {
#ifdef _DEBUG_MEMORY
  std::cout << "c delete constraint queue" << std::endl;
#endif
  triggers += min_priority;
  delete [] triggers;
}

void Mistral::ConstraintQueue::trigger(GlobalConstraint *cons)//;
{
// #ifdef _DEBUG_AC
//   std::cout << " initial trigger for " << c << "(" << (cons->id) << ")" << std::endl;
// #endif

  //int priority = cons->priority, cons_id = cons->id, triggered=false;
  Event evt;
  Variable x;

  
  for(unsigned int i=0; i<cons->scope.size; ++i) {
    x = cons->_scope[i];
    
    evt = (// cons->scope[i].domain_type != BOOL_VAR &&
	   x.is_ground() ? VALUE_EVENT : (LB_EVENT|UB_EVENT));
    
    if(cons->is_triggered_on(i, EVENT_TYPE(evt))) trigger(cons, i, evt);
  }
}




void Mistral::ConstraintQueue::trigger(BinaryConstraint *cons)//;
{
  add(cons);
  // int cons_id = cons->id;
  // if(!_set_.fast_contain(cons_id)) {
  //   _set_.fast_add(cons_id);
  //   triggers[2].add(cons_id);
  //   if(2 > higher_priority) higher_priority = 2;
  // }
}

void Mistral::ConstraintQueue::trigger(TernaryConstraint *cons)//;
{
  add(cons);
  // int cons_id = cons->id;
  // if(!_set_.fast_contain(cons_id)) {
  //   _set_.fast_add(cons_id);
  //   triggers[2].add(cons_id);
  //   if(2 > higher_priority) higher_priority = 2;
  // }
}

void Mistral::ConstraintQueue::add(ConstraintImplementation *cons)//;
{
  int cons_id = cons->id;
  if(!_set_.fast_contain(cons_id)) {
    _set_.fast_add(cons_id);
    triggers[2].add(cons_id);
    if(2 > higher_priority) higher_priority = 2;
  }
}

void Mistral::ConstraintQueue::add(Constraint cons)//;
{
  add(cons.propagator);
}


void Mistral::ConstraintQueue::trigger(GlobalConstraint *cons, const int var, const Event evt)//;
{

  //int var = c.index();
  //GlobalConstraint *cons = (GlobalConstraint*)c.propagator;

// #ifdef _DEBUG_AC
//   std::cout << "  triggers " << cons << " after a " 
// 	    << (ASSIGNED(evt) ? "value" : (RANGE_CHANGED(evt) ? "range" : "domain"))
// 	    << " event on "
// 	    << cons->scope[var] << " in " << cons->scope[var].get_domain() 
// 	    << std::endl;
// #endif

  if(cons != solver->taboo_constraint) {
    int priority = cons->priority, cons_id = cons->id;
    if(_set_.fast_contain(cons_id)) {
      cons->notify_other_event(var, evt);
      // if(cons->events.contain(var)) {
      // 	cons->event_type[var] |= evt;
      // } else {
      // 	cons->events.add(var);
      // 	cons->event_type[var] = evt;
      // }
    } else {

      //std::cout << "\nadd " << cons_id << " to " << _set_  << " " << _set_.table << std::endl;

      _set_.fast_add(cons_id);


      //std::cout << "here " << _set_ << " " << priority << std::endl;

      if(priority > higher_priority) higher_priority = priority;
      triggers[priority].add(cons_id);
      cons->notify_first_event(var, evt);
      // cons->events.set_to(var);
      // cons->event_type[var] = evt;
    }
  } 

#ifdef _DEBUG_QUEUE

  else
    std::cout << cons << "(" << (cons->id) 
	      << ") is currently being processed " 
	      << std::endl;
      
#endif


#ifdef _DEBUG_QUEUE

  std::cout << "=> " << cons->events << std::endl;

#endif

}


std::ostream& Mistral::ConstraintQueue::display(std::ostream& os) {
  int elt=INFTY;
  int end;

  os << "[";
  for(int i=cardinality-1; i>=0; --i) {
    if(!triggers[i+min_priority].empty()) {
      if(elt != INFTY) os << " | ";

      elt = triggers[i+min_priority].first();
      end = triggers[i+min_priority]._head;
      
      os << solver->constraints[elt] ;

      while(triggers[i+min_priority].next[elt] != end) {
	elt = triggers[i+min_priority].next[elt];
	os << " " << solver->constraints[elt] ;
      }
    }
  }
  os << "]";


  // for(int i=0; i<cardinality; ++i) {
  //   if(!triggers[i+min_priority].empty()) {
  //     elt = triggers[i+min_priority].first();
  //     end = triggers[i+min_priority]._head;
  //     os << "P" << (i+min_priority) << ": " 
  // 	//<< solver->constraints[elt];
  // 	 << "["<< solver->constraints[elt].id() << "]";

  //     if(!_set_.contain(solver->constraints[elt].id())) {
  // 	std::cout << "inconsistent constraint queue" <<std::endl;
  // 	exit(1);
  //     }

  //     while(triggers[i+min_priority].next[elt] != end) {
  // 	elt = triggers[i+min_priority].next[elt];
  // 	os << ", " //<< solver->constraints[elt];
  // 	   << " [" << solver->constraints[elt].id() << "]";

  // 	if(!_set_.contain(solver->constraints[elt].id())) {
  // 	  std::cout << "inconsistent constraint queue" <<std::endl;
  // 	  exit(1);
  // 	}
  //     }
  //     os << std::endl;
  //   }
  // }
  return os;
}

Mistral::Solver::Solver() 
#ifdef _MONITOR
  : monitor_list(this)
#endif
{ 
  consolidate_manager = NULL;
  search_started = false;

  // search stuf
  heuristic = NULL;
  policy = NULL;
  objective = NULL; //new Goal(Goal::SATISFACTION);
  //backjump_policy = NULL;

  // variables & constraints
  domain_types.initialise(0,128);
  variables.initialise(0,128);
  declared_variables.initialise(0,128);
  assignment_level.initialise(0,128);
  visited.initialise(0,1023);
  reason.initialise(0,128);
  constraints.initialise(0,256);
  //constraint_graph.initialise(128);
  posted_constraints.initialise(0,256,false);
  sequence.initialise(this);
  sequence.initialise(128);
  initialised_vars = 0;
  initialised_cons = 0;
  num_search_variables = 0;
  base = NULL;

  // trail stuff
  level = -1;
  //saved_objs.initialise(0,4096); 
  saved_vars.initialise(0,4096); 
  trail_.initialise    (0,4096);
  decisions.initialise (0,4096);
  // con_trail_.initialise(0,512);
  // con_trail_.add(0);
  // con_trail_.add(-1);

  active_variables.initialise(4096);

  // params
  parameters.initialise();

  // statistics
  statistics.initialise();

  heuristic = NULL; //new GenericHeuristic< GenericDVO< MinDomain >, MinValue >(this);
  policy = NULL; //new Geometric();

  usrand(parameters.seed);

  wiped_idx = CONSISTENT;

  save();
}

void Mistral::Solver::parse_dimacs(const char* filename) {
  unsigned int LARGENUMBER = 131072;
  std::ifstream infile( filename );
  char c=' ';
  std::string word;
  int N, M, l=0;
  Lit lit;
  Vector< Lit > new_clause;

  // skip comments
  infile >> c;
  while( c != 'p' ) {
    infile.ignore( LARGENUMBER, '\n' );
    infile >> c;
  }

  infile >> word;
  assert( word == "cnf" );
  
  // get number of atoms and clauses
  infile >> N;
  infile >> M;

  for(int i=0; i<N; ++i) {
    Variable x(0,1);
    add(x);
  }

  new_clause.initialise(0,N);
  // ConstraintClauseBase *cbase = new ConstraintClauseBase(variables);
  // base->reason = reason.stack_;
  // add(cbase);

  for(int i=0; i<M; ++i)
    {
      new_clause.clear();
      do {
	infile >> l;
	if(l!=0) {
	  if(l>0) lit = (l-1)*2+1;
	  else lit = (l+1)*-2;
	  new_clause.add(lit);
	  // if(parameters.init_activity == 1)
	  //   base->activity[lit] += parameters.activity_increment;
	}
      } while(l && infile.good());
      //cbase->add( new_clause );
      add( new_clause );

      //std::cout << new_clause << std::endl;
      
      //if(params.checked) add_original_clause( new_clause );
    }
  
  //init_watchers();
  
  //if(params.normalize_activity != 0)
  //normalize_activity(params.normalize_activity);
  
  //  std::cout << base << std::endl;

  //cbase->reason = reason.stack_;
}


void Mistral::Solver::add(Vector< Lit >& clause) {
  if(!base) {
    base = new ConstraintClauseBase(variables);
    add(base);
    base->reason = reason.stack_;
  }
  base->add( clause, (parameters.init_activity ? parameters.activity_increment : 0.0) );
 }

void Mistral::Solver::set_parameters(SolverParameters& p) {
  parameters = p;
}

void Mistral::Solver::add(VarArray& x) {
  for(unsigned int i=0; i<x.size; ++i)
    x[i].initialise(this);
}

void Mistral::Solver::add(Variable x) { 

  //std::cout << "add a variable " << x << std::endl;

  x.initialise(this); 
}

void Mistral::Solver::remove(Variable x) { 
  int idx = x.id(), i, j;
  domain_types[idx] |= REMOVED_VAR;
  for(Event trig = 0; trig<3; ++trig) {
    for(i = constraint_graph[idx].on[trig].size; i--;) {
      j = constraint_graph[idx].on[trig][i].id();
      if(posted_constraints.contain(j)) posted_constraints.remove(j);
    }
  }
}

int Mistral::Solver::declare(Variable x) {
  if(x.domain_type > DYN_VAR) booleans.add(&x);

  // add the variables to the set of vars
  active_variables.declare(variables.size);


  x.variable->id = variables.size;
  x.variable->solver = this;
  visited.extend(variables.size);
  variables.add(x);
  declared_variables.add(x);
  assignment_level.add(INFTY);
  reason.add(NULL);
  domain_types.add(DYN_VAR|(x.is_range() ? RANGE_VAR : 0));

  last_solution_lb.add(-INFTY);
  last_solution_ub.add( INFTY);
  
  ConstraintTriggerArray array;
  bool extend_struct = (constraint_graph.capacity == constraint_graph.size);
  constraint_graph.add(array);
  if(extend_struct) {
    constraint_graph.add(array);
    for(int i = constraint_graph.size-1; i>=0; --i) {
      for(int j = 0; j<3; ++j) {
	for(int k = constraint_graph[i].on[j].size-1; k>=0; --k) {
	  constraint_graph[i].on[j][k].re_link_to(&constraint_graph[i].on[j]);
	}
      }
    }
  }
  constraint_graph.back().initialise(4);


  notify_add_variable();

  return variables.size-1;
}


// void Mistral::Solver::add(Constraint* c) { 
//   for()
// }

void Mistral::Solver::add(Constraint c) { 

  if(c.id() < 0) {

    c.initialise(this);

    // get a name for the constraint and add it to the list
    c.set_id(constraints.size); 
    //constraints.declare(c);
    constraints.add(c);


    active_constraints.declare(c, this);

    //std::cout << "NOTIFY CONSTRAINT " << c << std::endl;
    notify_add_constraint(c);

    c.post(this);

  } else {

    //std::cout << "awaken" << std::endl;
    c.awaken();
    
  }
  

  // std::cout << "================\n" << active_constraints << "\n================" << std::endl;

  c.trigger();
  //active_constraints.trigger(c);

  // if(con_trail_.back() != level) {
  //   con_trail_.add(posted_constraints.size);
  //   con_trail_.add(level);
  // }
  
  // if(!posted_constraints.safe_contain(c->id)) {
  //   posted_constraints.extend(c->id);
  //   posted_constraints.add(c->id);
  // }

  // std::cout << c << ": ";
  // ((TernaryConstraint*)
  //c.propagator->print_active();
  // std::cout << std::endl;

  // std::cout << "================\n" << active_constraints << "\n================" << std::endl;

  if(level <= 0 && !posted_constraints.safe_contain(c.id())) 
    posted_constraints.safe_add(c.id());
}

Mistral::Outcome Mistral::Solver::solve() {
  BranchingHeuristic *heu = new GenericHeuristic <
    GenericDVO < 
      MinDomainOverWeight, 1,
      //PruningCountManager 
      FailureCountManager
      >,
    RandomMinMax 
    > (this); 
  RestartPolicy *pol = new Geometric();
  return depth_first_search(variables, heu, pol); 
  //return (search_outcome == SAT || search_outcome == OPT);
}


Mistral::Outcome Mistral::Solver::minimize(Variable X) {
  BranchingHeuristic *heu = new GenericHeuristic <
    GenericDVO < 
      MinDomainOverWeight, 1,
      //PruningCountManager 
      FailureCountManager
      >,
    RandomMinMax 
    > (this); 
  RestartPolicy *pol = new Geometric();
  Goal *goal = new Goal(Goal::MINIMIZATION, X.get_var());
  return depth_first_search(variables, heu, pol, goal); 
  //return (search_outcome == OPT);
}


Mistral::Outcome Mistral::Solver::maximize(Variable X) {
  BranchingHeuristic *heu = new GenericHeuristic <
    GenericDVO < 
      MinDomainOverWeight, 1,
      //PruningCountManager 
      FailureCountManager
      >,
    RandomMinMax 
    > (this); 
  RestartPolicy *pol = new Geometric();
  Goal *goal = new Goal(Goal::MAXIMIZATION, X.get_var());
  return depth_first_search(variables, heu, pol, goal); 
  //return (search_outcome == OPT);
}

Mistral::Outcome Mistral::Solver::depth_first_search(BranchingHeuristic *heu, 
						     RestartPolicy *pol,
						     Goal *goal) {
  return depth_first_search(variables, heu, pol, goal);
}

Mistral::Outcome Mistral::Solver::depth_first_search(Vector< Variable >& seq, 
						     BranchingHeuristic *heu, 
						     RestartPolicy *pol,
						     Goal *goal) 
{
  //std::cout << "\nINIT LEVEL = " << level << std::endl;
  initialise_search(seq, heu, pol, goal);

  return restart_search();
}
 

Mistral::Outcome Mistral::Solver::sequence_search(Vector< Vector< Variable > >& sequences,
						  Vector< BranchingHeuristic * >& heuristics,
						  Vector< RestartPolicy * >& policies,
						  Vector< Goal * >& goals
						  ) {
#ifdef _DEBUG_SEARCH
  std::cout << " c start new sequence search (in " << sequences.size << " phases)" << std::endl;
#endif

  unsigned int phase = 0;
  Outcome satisfiability = UNKNOWN, phase_satisfiability = UNKNOWN;
  Vector< int > phase_level;
  phase_level.add(level);

  // repeat until
  while(satisfiability == UNKNOWN) {  

#ifdef _DEBUG_SEARCH
    std::cout << " c";
    for(unsigned int i=0; i<phase; ++i) std::cout << "    ";
    std::cout << " init phase " << phase << std::endl;
#endif

    //initialise with the parameters of the current phase
    initialise_search(sequences[phase], heuristics[phase], policies[phase], goals[phase]);
 

#ifdef _DEBUG_SEARCH
    std::cout << " c";
    for(unsigned int i=0; i<phase; ++i) std::cout << "    ";
    std::cout << " search phase " << phase << ": ";
#endif

    //search the subset of variables
    if(objective->has_function()) {
      objective->set_type(phase < sequences.size-1 ? Goal::SATISFACTION : Goal::OPTIMIZATION);
    }
    phase_satisfiability = chronological_dfs(phase_level.back());//search(); //false, (phase < sequences.size-1));

#ifdef _DEBUG_SEARCH
    std::cout << outcome2str(phase_satisfiability) << std::endl;
#endif

    if(phase_satisfiability == UNSAT || phase_satisfiability == OPT) 
      {

	// the current phase is not satisfiable
	if(phase_level.empty()) 
	  {
#ifdef _DEBUG_SEARCH
	    std::cout << " c";
	    for(unsigned int i=0; i<phase; ++i) std::cout << "    ";
	    std::cout << " UNSAT! " << std::endl;
#endif

	    // we have exhausted the complete search, returns
	    satisfiability = UNSAT;
	  }
	else
	  {
	    // go back to the previous phase
	    phase_level.pop();
	    --phase;

#ifdef _DEBUG_SEARCH
	    std::cout << " c";
	    for(unsigned int i=0; i<phase; ++i) std::cout << "    ";
	    std::cout << " backtrack to phase " << phase << std::endl;
	    //exit(1);
#endif
	    
	    branch_right();

	  }
      }
    else if(phase_satisfiability == SAT)
      {

	phase_level.add(level);
	// the current phase has a solution
	if(++phase == sequences.size)
	  {

#ifdef _DEBUG_SEARCH
	    std::cout << " c";
	    for(unsigned int i=0; i<phase; ++i) std::cout << "    ";
	    std::cout << " SAT! " << std::endl;
#endif

	    // we have found a complete solution
	    satisfiability = SAT;
	  }

#ifdef _DEBUG_SEARCH
	else{
	  std::cout << " c";
	  for(unsigned int i=0; i<phase; ++i) std::cout << "    ";
	  std::cout << " advance to phase " << phase << std::endl;
	}
#endif

      }
  }

  statistics.outcome = satisfiability;

  statistics.end_time = get_run_time();

  if(parameters.verbosity)  {
    std::cout << statistics;
  }

  return satisfiability;
}

Mistral::Outcome Mistral::Solver::restart_search(const int root, const bool _restore_) { //const bool _restore_, const bool _exit_on_solution_) {

  //int initial_level = level; 

  Outcome satisfiability = UNKNOWN;

  statistics.objective_value = objective->value();

  while(satisfiability == UNKNOWN) {

    statistics.num_constraints = posted_constraints.size;
    statistics.num_variables = sequence.size;
    statistics.num_values = 0;
    for(int i=0; i<sequence.size; ++i)
      statistics.num_values += sequence[i].get_size();
    
    if(parameters.verbosity>1) {
      statistics.print_short(std::cout);
      std::cout << std::endl;
    }

    ++statistics.num_restarts;

    satisfiability = //(parameters.backjump ? 
      //conflict_directed_backjump() :
      chronological_dfs(root); //_exit_on_solution_); //);
    
    // if(_exit_on_solution_ && objective)
    //   satisfiability = objective->notify_solution(this);

    if(satisfiability == LIMITOUT) {

      policy->reset(parameters.restart_limit);
    
      if(!limits_expired()) {
	satisfiability = UNKNOWN;
      }
      forget();
    }

    if(_restore_) restore(root);
  }

  statistics.outcome = satisfiability;

  statistics.end_time = get_run_time();

  if(parameters.verbosity)  {
    std::cout << statistics;
  }

  return satisfiability;
}

Mistral::Outcome Mistral::Solver::get_next_solution()  
{
  Outcome satisfiability = UNSAT;

  //   std::cout << "get next solution " << (decisions.size) << " "
  // 	    << search_started << std::endl;
  
  if(search_started) {
    if(decisions.size) 
      branch_right();
    else return satisfiability;
  }
   
  search_started = true;
  
  statistics.num_variables = sequence.size;
  statistics.num_values = 0;
  for(int i=0; i<sequence.size; ++i)
    statistics.num_values += sequence[i].get_size();
  
  //display(std::cout);
  satisfiability = chronological_dfs();
  
  if(parameters.verbosity) {
    statistics.print_short(std::cout);
  }

  return satisfiability;
}

void Mistral::Solver::BooleanMemoryManager::add(Variable *x) {

  if(size.back() < 1024) {
    x->bool_domain = slots.back()+size.back();
    ++size.back();
  } else {
    int *nslot = new int[1024];
    std::fill(nslot, nslot+1024, 3);
    size.add(1);
    slots.add(nslot);
    x->bool_domain = nslot;
  }


  //std::cout << "zzz " << *x << ": " << x->domain_type << std::endl;
}


void Mistral::Solver::initialise_search(Vector< Variable >& seq, 
					BranchingHeuristic *heu, 
					RestartPolicy *pol,
					Goal *goal) 
{

  //std::cout << "INIT SEARCH!" << std::endl;

  consolidate();

  if(level < 0) save();

  active_solver = this;
  signal(SIGINT,Mistral_SIGINT_handler);


  sequence.clear();
  //decisions.clear();
  for(unsigned int i=seq.size; i;) {
    Variable x = seq[--i].get_var();
    if(!x.is_ground() && !sequence.contain(x) && !(domain_types[x.id()]&REMOVED_VAR)) sequence.add(x);
  }
  num_search_variables = sequence.size;


  if(heu) { // delete heuristic
      ; heuristic = heu; }
  else if(!heuristic) heuristic = new GenericHeuristic< Lexicographic, MinValue >(this);
  if(pol) { // delete policy;
    policy    = pol; }
  else if(!policy)    policy    = new NoRestart();
  if(goal){ // delete objective;
    objective = goal;}
  else if(!objective) objective = new Goal(Goal::SATISFACTION);

  // std::cout << (int*)heu << " " << (int*)heuristic << std::endl;
  // std::cout << heuristic << std::endl;
  // heuristic->display(std::cout);
  // std::cout << std::endl << sequence << std::endl;

  heuristic->initialise(sequence);

  parameters.restart_limit = policy->base;
  parameters.limit = (policy->base > 0);

  statistics.num_constraints = constraints.size;
  
  if(parameters.verbosity)  std::cout << " c +" << std::setw(90) << std::setfill('=')
  				      << "+" << std::endl << std::setfill(' ') 
  				      << " c |      INSTANCE STATS       |                    SEARCH STATS                 | OBJECTIVE |" << std::endl 
  				      << " c |   vars |    vals |   cons |    nodes | filterings | propagations | cpu time |           |" << std::endl;
}

  
Mistral::Solver::~Solver() {
#ifdef _DEBUG_MEMORY
  std::cout << "c delete solver" << std::endl;
#endif

  delete heuristic;
  delete policy;
  delete objective;

  //std::cout << "c delete consolidate manager" << std::endl;

  delete consolidate_manager;

  //std::cout << "delete constraints" << std::endl;
  for(unsigned int i=0; i<constraints.size; ++i) {

    //std::cout << "  delete " << constraints[i] << std::endl;

    delete constraints[i].propagator;
  }

  //std::cout << "delete expressions" << std::endl;
  for(unsigned int i=0; i<expression_store.size; ++i) {

    //Variable x(expression_store[i]);
    //std::cout << "  delete " << expression_store[i] << std::endl;

    delete expression_store[i];
  }

  //std::cout << "delete variables" << std::endl;
  for(unsigned int i=0; i<variables.size; ++i) {

    //std::cout << "  delete " << variables[i] << " in " << variables[i].get_domain() << std::endl;

    int domain_type = variables[i].domain_type;
    if     (domain_type ==  BITSET_VAR) delete variables[i].bitset_domain;
    else if(domain_type ==    LIST_VAR) delete variables[i].list_domain;
    else if(domain_type ==   RANGE_VAR) delete variables[i].range_domain;
    else if(domain_type == VIRTUAL_VAR) delete variables[i].virtual_domain;
    else if(domain_type ==  EXPRESSION) delete variables[i].expression;
    else if(domain_type !=   CONST_VAR) delete variables[i].variable;

// #ifdef _CONSTRAINT_LIST
//     delete constraint_graph[i].propagator;
// #endif

  }

}





// void Mistral::Solver::trigger_event(const int var, 
// 				    const Mistral::Event evt) {
  
//   active_variables.add(var, evt);
   
// }

// void Mistral::Solver::save(const int idx) {
//   saved_vars.add(idx);
// }

// void Mistral::Solver::() {

// }

void Mistral::Solver::restore() {

  unsigned int previous_level;
  //Constraint c;

  previous_level = trail_.pop();


#ifdef _DEBUG_RESTORE
  std::cout << "Restore to level " << previous_level << std::endl;
#endif

  while( saved_cons.size > previous_level ) {

#ifdef _DEBUG_RESTORE
    std::cout << "  (c) " << saved_cons.back() << " -> ";
#endif

    saved_cons.pop().restore();

#ifdef _DEBUG_RESTORE
    std::cout << "  (c) " << saved_cons.back(0) << std::endl;
#endif

  }

  previous_level = trail_.pop();
  while( saved_ints.size > previous_level ) {
    
#ifdef _DEBUG_RESTORE
    std::cout << "  (i) " << saved_ints.back() << " -> ";
#endif

    saved_ints.pop()->restore();

#ifdef _DEBUG_RESTORE
    std::cout << "  (i) " << saved_ints.back(0) << std::endl;
#endif

  }
  
  previous_level = trail_.pop();
  while( saved_lists.size > previous_level ) {

#ifdef _DEBUG_RESTORE
    std::cout << "  (l) " << *(saved_lists.back()) << " -> ";
#endif

    saved_lists.pop()->restore();

#ifdef _DEBUG_RESTORE
    std::cout << "  (l) " << *(saved_lists.back(0)) << std::endl;
#endif

  }
  
  previous_level = trail_.pop();
  while( saved_bools.size > previous_level ) {

#ifdef _DEBUG_RESTORE
    std::cout << "  (b) " << saved_bools.back() << " -> ";
#endif

    *(saved_bools.pop()) = 3;

#ifdef _DEBUG_RESTORE
    std::cout << "  (b) " << saved_bools.back(0) << std::endl;
#endif

  }

  previous_level = trail_.pop();
  while( saved_vars.size > previous_level ) {

#ifdef _DEBUG_RESTORE
    std::cout << "  (v) " << variables[saved_vars.back()] << " in " << variables[saved_vars.back()].get_domain() << " -> ";
#endif

    variables[saved_vars.pop()].restore();

#ifdef _DEBUG_RESTORE
    std::cout << "  (v) " << variables[saved_vars.back(0)] << " in " << variables[saved_vars.back(0)].get_domain() << std::endl;
#endif

  }

  --level;
  ++statistics.num_backtracks;


  // unsigned int previous_level;


  // previous_level = trail_.pop();
  // //std::cout << saved_relax.size << " -> " <<  previous_level << std::endl;

  // while( saved_relax.size > previous_level ) 
  //   saved_relax.pop()->post_on_array();  

  // previous_level = trail_.pop();
  // //std::cout << saved_post.size << " -> " <<  previous_level << std::endl;

  // while( saved_post.size > previous_level ) 
  //   saved_post.pop()->relax_from_array();  

  // previous_level = trail_.pop();
  // while( saved_vars.size > previous_level ) {
  //   // if(saved_vars.back() == 4) {
  //   //   std::cout << level << " RESTORE X4: " << variables[4] << " in " << variables[4].get_domain() 
  //   // 		<< " " << (variables[4].domain_type == RANGE_VAR ? 
  //   // 			   ((VariableRange*)(variables[4].variable))->trail_ :
  //   // 			   ((VariableBitmap*)(variables[4].variable))->trail_)
  //   // 		<< std::endl;
  //   // }

  //   variables[saved_vars.pop()].restore();

  //   // if(saved_vars.back(0) == 4) {
  //   //   std::cout << level << " ======> X4: " << variables[4] << " in " << variables[4].get_domain() 
  //   // 		<< " " << (variables[4].domain_type == RANGE_VAR ? 
  //   // 			   ((VariableRange*)(variables[4].variable))->trail_ :
  //   // 			   ((VariableBitmap*)(variables[4].variable))->trail_)
  //   // 		<< std::endl << std::endl;
  //   // }
  // }

  // previous_level = trail_.pop();
  // while( saved_objs.size > previous_level )
  //   saved_objs.pop()->restore();

  // if(con_trail_.back() == level) {
  //   con_trail_.pop();
  //   posted_constraints.size = con_trail_.pop();
  // }

  // //  decisions.pop(); 
  // sequence.size = trail_.pop();

  // ++statistics.num_backtracks;
  // --level;
}

void Mistral::Solver::restore(const int lvl) {
  decisions.size = lvl;
  while(lvl < level) restore();
}


void Mistral::Solver::add(Mistral::SolutionListener* l) {
  l->mid = solution_triggers.size;
  solution_triggers.add(l);
}
void Mistral::Solver::add(Mistral::RestartListener* l) {
  l->rid = restart_triggers.size;
  restart_triggers.add(l);
}
void Mistral::Solver::add(Mistral::SuccessListener* l) {
  l->sid = success_triggers.size;
  success_triggers.add(l);
}
void Mistral::Solver::add(Mistral::FailureListener* l) {
  l->fid = failure_triggers.size;
  failure_triggers.add(l);
}
void Mistral::Solver::add(Mistral::DecisionListener* l) {
  l->did = decision_triggers.size;
  decision_triggers.add(l);
}
void Mistral::Solver::add(Mistral::VariableListener* l) {
  l->vid = variable_triggers.size;
  variable_triggers.add(l);
}
void Mistral::Solver::add(Mistral::ConstraintListener* l) {
  l->cid = constraint_triggers.size;
  constraint_triggers.add(l);
}


void Mistral::Solver::remove(Mistral::SolutionListener* l) {
  unsigned int idx = l->mid;
  solution_triggers.remove(idx);
  if(solution_triggers.size>idx) 
    solution_triggers[idx]->mid = idx;
}
void Mistral::Solver::remove(Mistral::RestartListener* l) {
  unsigned int idx = l->rid;
  restart_triggers.remove(idx);
  if(restart_triggers.size>idx) 
    restart_triggers[idx]->rid = idx;
}
void Mistral::Solver::remove(Mistral::SuccessListener* l) {
  unsigned int idx = l->sid;
  success_triggers.remove(idx);
  if(success_triggers.size>idx) 
    success_triggers[idx]->sid = idx;
}
void Mistral::Solver::remove(Mistral::FailureListener* l) {
  unsigned int idx = l->fid;
  failure_triggers.remove(idx);
  if(failure_triggers.size>idx) 
    failure_triggers[idx]->fid = idx;
}
void Mistral::Solver::remove(Mistral::DecisionListener* l) {
  unsigned int idx = l->did;
  decision_triggers.remove(idx);
  if(decision_triggers.size>idx) 
    decision_triggers[idx]->did = idx;
}
void Mistral::Solver::remove(Mistral::VariableListener* l) {
  unsigned int idx = l->vid;
  variable_triggers.remove(idx);
  if(variable_triggers.size>idx) 
    variable_triggers[idx]->vid = idx;
}
void Mistral::Solver::remove(Mistral::ConstraintListener* l) {
  unsigned int idx = l->cid;
  constraint_triggers.remove(idx);
  if(constraint_triggers.size>idx) 
    constraint_triggers[idx]->cid = idx;
}


void Mistral::Solver::notify_failure() { //Constraint *con, const int idx) {
  for(unsigned int i=0; i<failure_triggers.size; ++i) {
    failure_triggers[i]->notify_failure();
  }
} 

void Mistral::Solver::notify_success() { //Variable* changes, const int n) {
  for(unsigned int i=0; i<success_triggers.size; ++i) {
    success_triggers[i]->notify_success();
  }
} 

void Mistral::Solver::notify_decision() { //Decision d) {
  for(unsigned int i=0; i<decision_triggers.size; ++i) {
    decision_triggers[i]->notify_decision();
  }
} 

void Mistral::Solver::notify_restart() { 
  for(unsigned int i=0; i<restart_triggers.size; ++i) {
    restart_triggers[i]->notify_restart();
  }
} 

void Mistral::Solver::notify_relax(Constraint c) { 
  for(unsigned int i=0; i<constraint_triggers.size; ++i) {
    constraint_triggers[i]->notify_relax(c);
  }
} 

void Mistral::Solver::notify_add_constraint(Constraint c) { 
  for(unsigned int i=0; i<constraint_triggers.size; ++i) {
    constraint_triggers[i]->notify_add_con(c);
  }
} 

void Mistral::Solver::notify_post(Constraint c) { 
  for(unsigned int i=0; i<constraint_triggers.size; ++i) {
    constraint_triggers[i]->notify_post(c);
  }
} 

void Mistral::Solver::notify_change_variable(const int idx) { 
  for(unsigned int i=0; i<variable_triggers.size; ++i) {
    variable_triggers[i]->notify_change(idx);
  }
} 

void Mistral::Solver::notify_add_variable() { 
  for(unsigned int i=0; i<variable_triggers.size; ++i) {
    variable_triggers[i]->notify_add_var();
  }
} 

void Mistral::Solver::consolidate() 
{
  if(!initialised_vars) {
    // std::cout << "CREATE CONSListener" << std::endl;
    
    // std::cout << variable_triggers.size << " " << constraint_triggers.size << std::endl;
    
    //ConsolidateListener *cl 
    consolidate_manager = new ConsolidateListener(this);
    //add((VariableListener*)consolidate_manager);
    //add((ConstraintListener*)consolidate_manager);
    
    // std::cout << variable_triggers.size << " " << constraint_triggers.size << std::endl;
    
    
    // std::cout << "consolidate_manager->constraints" << std::endl << consolidate_manager->constraints << std::endl;
  }

  for(; initialised_vars<variables.size; ++initialised_vars) {
    variables[initialised_vars] = variables[initialised_vars].get_var();
    if(!(domain_types[initialised_vars]&RANGE_VAR) 
       && variables[initialised_vars].domain_type == RANGE_VAR
       && !variables[initialised_vars].is_ground() 
       && variables[initialised_vars].get_degree()>0) {

      //std::cout << "change " << domain_types[initialised_vars] << std::endl;

      int minval = variables[initialised_vars].get_min();
      int maxval = variables[initialised_vars].get_max();
      Variable X(minval, 
		 maxval,
		 domain_types[initialised_vars]);

      X.variable->solver = this;
      X.variable->id = initialised_vars;
      variables[initialised_vars] = X;

      if(X.domain_type > DYN_VAR) {
	booleans.add(variables.stack_+initialised_vars);
      }
    }
  }

  while(initialised_cons < constraints.size)
    constraints[initialised_cons++].consolidate();

}


void Mistral::Solver::make_non_convex(const int idx) 
{

  // std::cout << variables << std::endl;
  // std::cout << "\nmake " << variables[idx] << " non convex\n"; 


  if(variables[idx].domain_type == RANGE_VAR) {
    Variable X(variables[idx], true);
    
    // if(variables[idx].is_expression()) {

    //   std::cout << "(exp)" << std::endl;

    //   Variable x = variables[idx];
    //   while (x.expression->self.is_expression()) {
    // 	x = x.expression->self;
    //   }
    //   x.expression->self = X;

    // } else {
      variables[idx] = X;


    // int ids = sequence.index(idx);

    // if(ids>=0) sequence.list_[ids] = X;

    // for(int i=0; i<3; ++i) {
    //   for(int j=constraint_graph[idx].on[i].size-1; j>=0; --j)
    // 	constraint_graph[idx].on[i][j].consolidate();
    // }

    //std::cout << "MAKE " << variables[idx] << " NON CONVEX: NOTIFY CHANGE" << std::endl; 

      notify_change_variable(idx);
      // }

    // std::cout << "c" << std::endl;
    // unsigned int k=0;
    // // for(; k<monitored_variables.size; ++k) {
    // //   monitored_variables[k] = monitored_variables[k].get_var();
    // // }
    // // k = 0;
    // for(unsigned int q=0; q<monitored_index.size; ++q) {
    //   std::cout << "c "; for(int lvl=0; lvl<level; ++lvl) std::cout << " ";
    //   for(; k<monitored_index[q]; ++k) {
    // 	std::cout << "|" << variables[monitored[k]] ;
    // 	std::cout.flush();
    // 	std::cout << variables[monitored[k]].get_domain();
    // 	std::cout.flush();
    // 	std::cout << variables[monitored[k]].get_history() << " ";
    //   }
    //   std::cout << std::endl;
    // }


  }
}



bool Mistral::Solver::rewrite() 
{
  
  int i=0;
  unsigned int con_i=0;
  IntStack to_rewrite(0, constraints.size-1, false);
  Constraint con;
  RewritingOutcome rewritten;
  //bool fix_point;
  //Vector<Constraint> to_rewrite;



  //fix_point = 

  // enforce AC
  //while( !fix_point ) {

#ifdef _DEBUG_REWRITE
    std::cout << "\n===================START REWRITING=====================\n" ;
#endif

  do {

#ifdef _DEBUG_REWRITE
    std::cout << " propagate  " ;
#endif


    if( !propagate() ) break;

#ifdef _DEBUG_REWRITE
    std::cout << (*this) << std::endl;
    //std::cout << "Collect rewritable constraints: " << std::endl;
#endif

    // add the unseen constraints that might be rewritten
    for(; con_i<constraints.size; ++con_i) {

// #ifdef _DEBUG_REWRITE
//       std::cout << "   " << constraints[con_i] ;
// #endif

      if(constraints[con_i].rewritable()) {
	to_rewrite.safe_add(con_i);
	
// #ifdef _DEBUG_REWRITE
// 	std::cout << " in" << std::endl;
// #endif
	 
      } 

// #ifdef _DEBUG_REWRITE
//       else {
// 	std::cout << " out" << std::endl;
//       }
// #endif

    }

     
    // // add all rewritable constraints to the stack
    // for(i=0; i<to_rewrite.size; ++i)
    //   active_constraints.add(constraints[to_rewrite[i]]);
   

#ifdef _DEBUG_REWRITE
    std::cout << " rewrite: " << to_rewrite << std::endl;
#endif


    // // rewriting 
    // while(!active_constraints.empty()) {
     
    //   con = active_constraints.select(constraints);
    for(i=to_rewrite.size; i--;) {
      con = constraints[to_rewrite[i]];


#ifdef _DEBUG_REWRITE
      std::cout << "   [rewrite " << con << std::endl;
#endif

      rewritten = con.rewrite();
       
      switch(rewritten) {
      case NO_EVENT: {

#ifdef _DEBUG_REWRITE
	std::cout << "    -> no event] " << std::endl;
#endif

      }
	break;
	 
      case SUPPRESSED: { 

#ifdef _DEBUG_REWRITE
	std::cout << "    -> suppressed] " << std::endl;
#endif

	to_rewrite.remove(con.id());
	if(posted_constraints.contain(con.id())) posted_constraints.remove(con.id());

	// #ifdef _DEBUG_REWRITE
	// 	 std::cout << to_rewrite << std::endl;
	// 	 std::cout << posted_constraints << std::endl;
	// #endif


	  } break;
      default: { 


#ifdef _DEBUG_REWRITE
	std::cout << "   -> replaced] " << std::endl;
	exit(1);
#endif

	to_rewrite.remove(con.id()); 
	if(constraints[rewritten].rewritable()) {
	  to_rewrite.add(rewritten);
	  active_constraints.add(constraints[rewritten]);
	}
      }
      }
    }


#ifdef _DEBUG_REWRITE

    std::cout << "==>\n" << (*this) << std::endl;

#endif

#ifdef _DEBUG_REWRITE
    std::cout << " propagate: " << active_variables << std::endl;
    std::cout << "       and: " << active_constraints << std::endl;
#endif

      //fix_point = active_variables.empty();
  } while (!active_variables.empty() || !active_constraints.empty());


#ifdef _DEBUG_REWRITE
  std::cout << "\n====================END REWRITING======================\n" ;
#endif

  
  bool consistent = IS_OK(wiped_idx);
  wiped_idx = CONSISTENT;
  return consistent;
    
  //return IS_OK(wiped_idx);

}


Mistral::PropagationOutcome Mistral::Solver::propagate(Constraint c, 
						       const bool force_trigger,
						       const bool trigger_self) {

  // std::cout << "solver.cpp: specific propagate(" << c << ")" << std::endl;

  // std::cout << active_variables << std::endl
  // 	    << active_constraints << std::endl;

  int trig, cons;
  bool fix_point;
  Triplet < int, Event, ConstraintImplementation* > var_evt;

  //wiped_idx = CONSISTENT;
  if(force_trigger) 
    c.trigger();
  
  fix_point = (active_variables.empty() && active_constraints.empty());

  // std::cout << IS_OK(wiped_idx) << " & " << fix_point << std::endl;

  // std::cout << active_variables << std::endl;

  // std::cout << active_constraints << std::endl;

  while(IS_OK(wiped_idx) && !fix_point) {

    // empty the var stack first
    while( IS_OK(wiped_idx) && 
	   !active_variables.empty() ) {
    
      var_evt = active_variables.pop_front();

      if(ASSIGNED(var_evt.second) && sequence.contain(variables[var_evt.first])) {
	sequence.remove(variables[var_evt.first]);
	assignment_level[var_evt.first] = level;
      }


   
      // std::cout << var_evt << " " 
      // 		<< variables[var_evt.first] << " \n0 " 
      // 		<< constraint_graph[var_evt.first].on[0] << "\n1 "
      // 		<< constraint_graph[var_evt.first].on[1] << "\n2 "
      // 		<< constraint_graph[var_evt.first].on[2] << "\n"
      // 		<< c.propagator << std::endl;

     
      if(trigger_self || var_evt.third != c.propagator) {

	// for each triggered constraint
	for(trig = EVENT_TYPE(var_evt.second); IS_OK(wiped_idx) &&
	      trig<3; ++trig) {

	  for(cons = constraint_graph[var_evt.first].on[trig].size; IS_OK(wiped_idx) &&
		--cons>=0;) {

	    culprit = constraint_graph[var_evt.first].on[trig][cons];
	    
	    if( culprit == c ) {

	      // if the constraints asked to be pushed on the constraint stack we do that
	      if(culprit.pushed()) {
		active_constraints.trigger((GlobalConstraint*)culprit.propagator, 
					   culprit.index(), var_evt.second);
	      }

	      // if the constraint is not postponed, we propagate it
	      if(!culprit.postponed()) {
		++statistics.num_propagations;  
		taboo_constraint = culprit.freeze();
		wiped_idx = culprit.propagate(var_evt.second); 
		taboo_constraint = culprit.defrost();
	      }
	    } 
	  }
	}
      }
    }

    if(IS_OK(wiped_idx) && !active_constraints.empty()) {
      // propagate postponed constraint
      ++statistics.num_propagations;  
      culprit = active_constraints.select(constraints);
      taboo_constraint = culprit.freeze();
      wiped_idx = culprit.propagate(); 
      taboo_constraint = culprit.defrost();
    } else if(active_variables.empty()) fix_point = true;
  }
    
  taboo_constraint = NULL;
  active_constraints.clear();
  active_variables.clear();

  // std::cout << wiped_idx << std::endl;
  PropagationOutcome consistent = wiped_idx;
  wiped_idx = CONSISTENT;
  return consistent;
}

Mistral::PropagationOutcome Mistral::Solver::checker_propagate(Constraint c, 
							       const bool force_trigger,
							       const bool trigger_self) {
  int trig, cons;
  bool fix_point;
  Triplet < int, Event, ConstraintImplementation* > var_evt;


#ifdef _DEBUG_AC
  std::cout << "c start (checker) propagation" << std::endl;
#endif


  //wiped_idx = CONSISTENT;
  if(force_trigger) 
    c.trigger();
  
  fix_point = (active_variables.empty() && active_constraints.empty());


#ifdef _DEBUG_AC
  int iteration = 0;
  std::cout << std::endl;
#endif

  while(IS_OK(wiped_idx) && !fix_point) {

#ifdef _DEBUG_AC
    std::cout << "c "; 
    std::cout << "var stack: " << active_variables << std::endl;
    ++iteration;
#endif

    // empty the var stack first
    while( IS_OK(wiped_idx) && 
	   !active_variables.empty() ) {
    
      var_evt = active_variables.pop_front();

#ifdef _DEBUG_AC
      std::cout << "c "; //for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
      std::cout << "react to " << event2str(var_evt.second) << " on " << variables[var_evt.first] 
		<< " in " << variables[var_evt.first].get_domain() ;
      if(var_evt.third)
	std::cout << " because of " << var_evt.third ;
      std::cout << ". var stack: " << active_variables << std::endl;
#endif 

      if(ASSIGNED(var_evt.second) && sequence.contain(variables[var_evt.first])) {
	sequence.remove(variables[var_evt.first]);
	assignment_level[var_evt.first] = level;
      }
     
      //std::cout << var_evt.third << " <-> " << c.propagator << std::endl;

      if(trigger_self || var_evt.third != c.propagator) {

	// std::cout << "cons list of " << variables[var_evt.first] << " = " 
	// 	  << constraint_graph[var_evt.first].on << std::endl;

	// for each triggered constraint
	for(trig = EVENT_TYPE(var_evt.second); IS_OK(wiped_idx) &&
	      trig<3; ++trig) {

	  //std::cout << " trig[" << trig << "] = " << constraint_graph[var_evt.first].on[trig] << std::endl;

	  for(cons = constraint_graph[var_evt.first].on[trig].size; IS_OK(wiped_idx) &&
		--cons>=0;) {

	    culprit = constraint_graph[var_evt.first].on[trig][cons];


	    // if(ASSIGNED(var_evt.second)) {
	    //   culprit.notify_assignment();
	    // }


#ifdef _DEBUG_AC
	    std::cout << "c "; //for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
	    std::cout << "  -awake " << culprit << ": "; 
	    std::cout.flush();
#endif

	    
	    if( culprit == c ) {

	      // if the constraint asks to be pushed on the constraint stack we do that
	      if(culprit.pushed()) {
#ifdef _DEBUG_AC
	      std::cout << "pushed on the stack" ;
#endif
		active_constraints.trigger((GlobalConstraint*)culprit.propagator, 
					   culprit.index(), var_evt.second);
	      }

	      // if the constraint is not postponed, we propagate it
	      if(!culprit.postponed()) {
#ifdef _DEBUG_AC
	      if(IS_OK(wiped_idx)) {
		Variable *scp = culprit.get_scope();
		int arity = culprit.arity();
		for(int i=0; i<arity; ++i)
		  std::cout << scp[i].get_domain() << " ";
		std::cout << "ok";
	      } else std::cout << "fail";
#endif
		++statistics.num_propagations;  
		taboo_constraint = culprit.freeze();

		//std::cout << "taboo: " << taboo_constraint << std::endl;

		wiped_idx = culprit.checker_propagate(var_evt.second); 
		taboo_constraint = culprit.defrost();
	      }
	    }
#ifdef _DEBUG_AC
	    else {
	      std::cout << "c "; //for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
	      std::cout << "  -does not propagate " << culprit << " (we propagate " << c << ")" ;
	    }
	    std::cout << std::endl; 
#endif    

	  }
	}
      }
#ifdef _DEBUG_AC
	  else {
	    std::cout << "c "; //for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
	    std::cout << "  -does not awake " << culprit << " (idempotent)" << std::endl; 
	  }
#endif
    }
    
    if(IS_OK(wiped_idx) && !active_constraints.empty()) {
      // propagate postponed constraint
      culprit = active_constraints.select(constraints);
      taboo_constraint = culprit.freeze();

	      // if(taboo_constraint)
	      // 	std::cout << "[constraint]" << std::endl;
	      // else
	      // 	std::cout << "[nill]" << std::endl;

      wiped_idx = culprit.checker_propagate(); 
      taboo_constraint = culprit.defrost();
    } else if(active_variables.empty()) fix_point = true;
  }
    
  taboo_constraint = NULL;
  active_constraints.clear();
  active_variables.clear();


#ifdef _DEBUG_AC
  std::cout << "c end (checker) propagation\n" << std::endl;
#endif

    
  //return wiped_idx;
  PropagationOutcome consistent = wiped_idx;
  wiped_idx = CONSISTENT;
  return consistent;
}

Mistral::PropagationOutcome Mistral::Solver::bound_checker_propagate(Constraint c, 
								     const bool force_trigger,
								     const bool trigger_self) {


  //std::cout << "solver.cpp: bound checker propagate(" << c << ")" << std::endl;

  // std::cout << active_variables << std::endl
  // 	    << active_constraints << std::endl;


  int trig, cons;
  bool fix_point;
  Triplet < int, Event, ConstraintImplementation* > var_evt;
  //VarEvent var_evt;

  //wiped_idx = CONSISTENT;
  if(force_trigger) 
    c.trigger();

  fix_point =  (active_variables.empty() && active_constraints.empty());
  
  // std::cout << active_variables << std::endl;

  // std::cout << active_constraints << std::endl;

  // std::cout << (IS_OK(wiped_idx)) << " " << fix_point <<std::endl;


  while(IS_OK(wiped_idx) && !fix_point) {

    // empty the var stack first
    while( IS_OK(wiped_idx) && 
	   !active_variables.empty() ) {
    
      var_evt = active_variables.pop_front();

      if(ASSIGNED(var_evt.second) && sequence.contain(variables[var_evt.first])) {
	sequence.remove(variables[var_evt.first]);
	assignment_level[var_evt.first] = level;
      }
     
      // std::cout << var_evt << " " 
      // 		<< variables[var_evt.first] << " \n0 " 
      // 		<< constraint_graph[var_evt.first].on[0] << "\n1 "
      // 		<< constraint_graph[var_evt.first].on[1] << "\n2 "
      // 		<< constraint_graph[var_evt.first].on[2] << "\n"
      // 		<< c.propagator << std::endl;

      if(trigger_self || var_evt.third != c.propagator) {

	// std::cout << " " << constraint_graph[var_evt.first].on[EVENT_TYPE(var_evt.second)] << std::endl;

	// for each triggered constraint
	for(trig = EVENT_TYPE(var_evt.second); IS_OK(wiped_idx) &&
	      trig<3; ++trig) {

	  for(cons = constraint_graph[var_evt.first].on[trig].size; IS_OK(wiped_idx) &&
		--cons>=0;) {

	    culprit = constraint_graph[var_evt.first].on[trig][cons];


	    //std::cout << culprit << std::endl;
	    
	    if( culprit == c ) {

	      // if the constraints asked to be pushed on the constraint stack we do that
	      if(culprit.pushed()) {
		active_constraints.trigger((GlobalConstraint*)culprit.propagator, 
					   culprit.index(), var_evt.second);
	      }

	      // if the constraint is not postponed, we propagate it
	      if(!culprit.postponed()) {
		++statistics.num_propagations;  
		taboo_constraint = culprit.freeze();

	      // if(taboo_constraint)
	      // 	std::cout << "[constraint]" << std::endl;
	      // else
	      // 	std::cout << "[nill]" << std::endl;

		wiped_idx = culprit.bound_checker_propagate(var_evt.second); 
		taboo_constraint = culprit.defrost();
	      }
	    } 
	  }
	}
      }
    }

    // std::cout << active_constraints << std::endl;
    // std::cout << active_constraints.empty() << std::endl;

    if(IS_OK(wiped_idx) && !active_constraints.empty()) {
      // propagate postponed constraint
      culprit = active_constraints.select(constraints);
      taboo_constraint = culprit.freeze();


	      // if(taboo_constraint)
	      // 	std::cout << "[constraint]" << std::endl;
	      // else
	      // 	std::cout << "[nill]" << std::endl;

      //std::cout << "solver.cpp: bound checker prop" << std::endl;

      wiped_idx = culprit.bound_checker_propagate(); 
      taboo_constraint = culprit.defrost();
    } else if(active_variables.empty()) fix_point = true;
  }
    
  taboo_constraint = NULL;
  active_constraints.clear();
  active_variables.clear();
    
  // std::cout << wiped_idx << std::endl;

  //return wiped_idx;
  PropagationOutcome consistent = wiped_idx;
  wiped_idx = CONSISTENT;
  return consistent;
}


// Mistral::PropagationOutcome Mistral::Solver::propagate(Constraint *cons) {
//   culprit = cons;
//   //active_constraints.taboo_constraint = culprit->freeze();
//   active_constraints.select(culprit);
//   wiped_idx = culprit->propagate();
//   culprit->defrost();
//   if(IS_OK(wiped_idx)) culprit = NULL;
//   return wiped_idx;
// }

//#define _DEBUG_AC true

bool Mistral::Solver::propagate() 
{

#ifdef _DEBUG_AC
  std::cout << "c start propagation" << std::endl;
#endif

  bool fix_point;
  int trig, cons;
  Triplet < int, Event, ConstraintImplementation* > var_evt;

  //wiped_idx = CONSISTENT;
  culprit.clear();

  ++statistics.num_filterings;  

  // TODO, we shouldn't have to do that
  if(IS_OK(wiped_idx) && objective && objective->enforce())
    wiped_idx = objective->objective.id();
  
  fix_point =  (active_variables.empty() && active_constraints.empty());


#ifdef _DEBUG_AC
  int iteration = 0;
  std::cout << std::endl;
#endif

  // if(!IS_OK(wiped_idx)) {

  //   std::cout << "FAILED WHEN ENFORCING UPPER BOUND" << std::endl;

  // }


  while(IS_OK(wiped_idx) && !fix_point) {
    
#ifdef _DEBUG_AC

    std::cout << "c "; //for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
    std::cout << "var stack: " << active_variables << std::endl;
    // std::cout << "c "; for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
    // std::cout << "con stack: " << active_constraints << std::endl
    //   ;

    ++iteration;
#endif

    // empty the var stack first
    while( IS_OK(wiped_idx) && 
	   !active_variables.empty() ) {
      
      // get the variable event
      var_evt = active_variables.pop_front();

#ifdef _DEBUG_AC
      std::cout << "c "; //for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
      std::cout << "react to " << event2str(var_evt.second) << " on " << variables[var_evt.first] 
		<< " in " << variables[var_evt.first].get_domain() ;
      if(var_evt.third)
	std::cout << " because of " << var_evt.third ;
      std::cout << ". var stack: " << active_variables << std::endl;
#endif      

      if(ASSIGNED(var_evt.second) && sequence.contain(variables[var_evt.first])) {
	sequence.remove(variables[var_evt.first]);
	assignment_level[var_evt.first] = level;
      }


	// std::cout << "here 0" << variables[var_evt.first] << ": " << constraint_graph[var_evt.first].on[0] << std::endl
	//  	  << constraint_graph[var_evt.first].on[0].size << std::endl;
	// std::cout << "here 1" << variables[var_evt.first] << ": " << constraint_graph[var_evt.first].on[1] << std::endl
	//  	  << constraint_graph[var_evt.first].on[1].size << std::endl;
	// std::cout << "here 2" << variables[var_evt.first] << ": " << constraint_graph[var_evt.first].on[2] << std::endl
	//  	  << constraint_graph[var_evt.first].on[2].size << std::endl;
      

      // for each triggered constraint
      for(trig = EVENT_TYPE(var_evt.second); IS_OK(wiped_idx) &&
	    trig<3; ++trig) {

	for(cons = constraint_graph[var_evt.first].on[trig].size; IS_OK(wiped_idx) &&
	      --cons>=0;) {

	  culprit = constraint_graph[var_evt.first].on[trig][cons];


	  if(ASSIGNED(var_evt.second)) {
	    culprit.notify_assignment();
	  }
	  
	  // idempotency, if the event was triggered by itself
	  if(var_evt.third != culprit.propagator) {
#ifdef _DEBUG_AC
	    std::cout << "c "; //for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
	    std::cout << "  -awake " << culprit << ": "; 
	    std::cout.flush();
#endif
	    // if the constraints asked to be pushed on the constraint stack we do that
	    if(culprit.pushed()) {
#ifdef _DEBUG_AC
	      std::cout << "pushed on the stack" ;
#endif    
	      active_constraints.trigger((GlobalConstraint*)culprit.propagator, 
					 culprit.index(), var_evt.second);
	    }
	    // if the constraint is not postponed, we propagate it
	    if(!culprit.postponed()) {
#ifdef _DEBUG_AC
	      if(culprit.pushed()) std::cout << ", ";
	      std::cout << "propagated: ";
	      Variable *scp = culprit.get_scope();
	      int arity = culprit.arity();
	      for(int i=0; i<arity; ++i)
		std::cout << scp[i].get_domain() << " ";
	      std::cout << "-> ";
	      std::cout.flush();
#endif
	      ++statistics.num_propagations;  
	      taboo_constraint = culprit.freeze();

	      // if(culprit.id() == 11) {
	      // 	std::cout << "PROPAGATE " << culprit << std::endl;
	      // }

	      wiped_idx = culprit.propagate(var_evt.second); 
	      taboo_constraint = culprit.defrost();
#ifdef _DEBUG_AC
	      if(IS_OK(wiped_idx)) {
		Variable *scp = culprit.get_scope();
		int arity = culprit.arity();
		for(int i=0; i<arity; ++i)
		  std::cout << scp[i].get_domain() << " ";
		std::cout << "ok";
	      } else std::cout << "fail";
#endif
	    }
#ifdef _DEBUG_AC
	    std::cout << std::endl; 
#endif    
	  } 
#ifdef _DEBUG_AC
	  else {
	    std::cout << "c "; //for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
	    std::cout << "  -does not awake " << culprit << " (idempotent)" << std::endl; 
	  }
#endif
	}
      }
    }


// #ifdef _DEBUG_AC
//     //std::cout << std::endl;
//     std::cout << "c "; //for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
//     //std::cout << " var stack: " << active_variables << std::endl;
//     //std::cout << "c2 "; for(int lvl=0; lvl<level; ++lvl) std::cout << " ";
//     //std::cout << "con stack: " << active_constraints << std::endl
//       ;
// #endif


    
    if(IS_OK(wiped_idx) && !active_constraints.empty()) {

#ifdef _DEBUG_AC
      if(level > 0) {
	std::cout << "\nc propagate postponed constraints: " 
		  << active_constraints << std::endl;
      } else {
	std::cout << "\nc propagate postponed constraints: "
		  << active_constraints._set_ << std::endl;
      }
#endif

      // propagate postponed constraint
      culprit = active_constraints.select(constraints);

#ifdef _DEBUG_AC
      std::cout << "c "; //for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
      std::cout << "  -propagate " << culprit << " (" ;

      if(culprit.global()) {
	GlobalConstraint *gc = (GlobalConstraint*)(culprit.propagator);
		
	std::cout << gc->events << " ";
	std::cout.flush();
	
	std::cout << event2str(gc->event_type[gc->events[0]]) << " on " << gc->scope[gc->events[0]] ;
	for(unsigned int i=1; i<gc->events.size; ++i) {
	  std::cout << ", " << event2str(gc->event_type[gc->events[i]]) << " on " << gc->scope[gc->events[i]] ;
	}
      }
      std::cout << ") ";
      Variable *scp = culprit.get_scope();
      int arity = culprit.arity();
      for(int i=0; i<arity; ++i)
	std::cout << scp[i].get_domain() << " ";
      std::cout << "-> ";
      std::cout.flush();
#endif

      ++statistics.num_propagations;  
      taboo_constraint = culprit.freeze();
      wiped_idx = culprit.propagate(); 
      taboo_constraint = culprit.defrost();

#ifdef _DEBUG_AC
      if(IS_OK(wiped_idx)) {
	Variable *scp = culprit.get_scope();
	int arity = culprit.arity();
	for(int i=0; i<arity; ++i)
	  std::cout << scp[i].get_domain() << " ";
	std::cout << "ok";
      } else std::cout << "fail";
      std::cout << std::endl;
#endif	
      
    } else if(active_variables.empty()) fix_point = true;
  }
    
  taboo_constraint = NULL;
  active_constraints.clear();
  if(!parameters.backjump) {
    active_variables.clear();
  }

#ifdef _DEBUG_AC
  if(!IS_OK(wiped_idx)) {
    std::cout << "inconsistency found!" << std::endl;
  } else {
    std::cout << "done" << std::endl;
  }
#endif 


#ifdef _DEBUG_AC
  std::cout << "c end propagation" << std::endl;
#endif

  
  if(IS_OK(wiped_idx)) {
    notify_success();
    return true;
  } else {
    ++statistics.num_failures;

    //std::cout << "solver: notify failure" << std::endl;

    notify_failure();
    wiped_idx = CONSISTENT;
    return false;
  }
}


void Mistral::Solver::fail() {
  wiped_idx = FAILURE(0);
}

// bool Mistral::Solver::rewrite() 
// {

//   bool fix_point;
//   int trig, cons;
//   Triplet < int, Event, ConstraintImplementation* > var_evt;

//   wiped_idx = CONSISTENT;
//   culprit.clear();

//   ++statistics.num_filterings;  

//   // TODO, we shouldn't have to do that
//   if(objective && objective->enforce())
//     wiped_idx = objective->objective.id();

//   fix_point =  (active_variables.empty() && active_constraints.empty());


// #ifdef _DEBUG_AC
//   int iteration = 0;
//   std::cout << std::endl;
// #endif

//   while(IS_OK(wiped_idx) && !fix_point) {
    
// #ifdef _DEBUG_AC

//     std::cout << "c "; for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
//     std::cout << "var stack: " << active_variables << std::endl;
//     // std::cout << "c "; for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
//     // std::cout << "con stack: " << active_constraints << std::endl
//     //   ;

//     ++iteration;
// #endif

//     // empty the var stack first
//     while( IS_OK(wiped_idx) && 
// 	   !active_variables.empty() ) {
      
//       // get the variable event
//       var_evt = active_variables.pop_front();

// #ifdef _DEBUG_AC
//       std::cout << "c "; for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
//       std::cout << "react to " << event2str(var_evt.second) << " on " << variables[var_evt.first] 
// 		<< " in " << variables[var_evt.first] ;
//       if(var_evt.third)
// 	std::cout << " because of " << var_evt.third ;
//       std::cout << ". var stack: " << active_variables << std::endl;
// #endif      


//       if(ASSIGNED(var_evt.second) && sequence.contain(variables[var_evt.first])) {
// 	sequence.remove(variables[var_evt.first]);
// 	assignment_level[var_evt.first] = level;
//       }
      

//       // for each triggered constraint
//       for(trig = EVENT_TYPE(var_evt.second); IS_OK(wiped_idx) &&
// 	    trig<3; ++trig) {

// 	// std::cout << variables[var_evt.first] << ": " << constraint_graph[var_evt.first].on[trig] << std::endl
// 	// 	  << constraint_graph[var_evt.first].on[trig].size << std::endl;

// 	for(cons = constraint_graph[var_evt.first].on[trig].size; IS_OK(wiped_idx) &&
// 	      --cons>=0;) {

// 	  culprit = constraint_graph[var_evt.first].on[trig][cons];
	  
// 	  // idempotency, if the event was triggered by itself
// 	  if(var_evt.third != culprit.propagator) {
// #ifdef _DEBUG_AC
// 	    std::cout << "c "; for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
// 	    std::cout << "  -awake " << culprit << ": "; 
// #endif
// 	    // if the constraints asked to be pushed on the constraint stack we do that
// 	    if(culprit.pushed()) {
// #ifdef _DEBUG_AC
// 	      std::cout << "pushed on the stack" ;
// #endif    
// 	      active_constraints.trigger((GlobalConstraint*)culprit.propagator, 
// 					 culprit.index(), var_evt.second);
// 	    }
// 	    // if the constraint is not postponed, we propagate it
// 	    if(!culprit.postponed()) {
// #ifdef _DEBUG_AC
// 	      if(culprit.pushed()) std::cout << ", ";
// 	      std::cout << "propagated: ";
// 	      Variable *scp = culprit.get_scope();
// 	      int arity = culprit.arity();
// 	      for(int i=0; i<arity; ++i)
// 		std::cout << scp[i].get_domain() << " ";
// 	      std::cout << "-> ";
// #endif
// 	      ++statistics.num_propagations;  
// 	      taboo_constraint = culprit.freeze();
// 	      wiped_idx = culprit.propagate(var_evt.second); 
// 	      taboo_constraint = culprit.defrost();

// 	      if(IS_OK(wiped_idx)) {
// 		culprit.rewrite();
// 	      }

// #ifdef _DEBUG_AC
// 	      if(IS_OK(wiped_idx)) {
// 		Variable *scp = culprit.get_scope();
// 		int arity = culprit.arity();
// 		for(int i=0; i<arity; ++i)
// 		  std::cout << scp[i].get_domain() << " ";
// 		std::cout << "ok";
// 	      } else std::cout << "fail";
// #endif
// 	    }
// #ifdef _DEBUG_AC
// 	    std::cout << std::endl; 
// #endif    
// 	  } 
// #ifdef _DEBUG_AC
// 	  else {
// 	    std::cout << "c "; for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
// 	    std::cout << "  -does not awake " << culprit << " (idempotent)" << std::endl; 
// 	  }
// #endif
// 	}
//       }
//     }


// #ifdef _DEBUG_AC
//     //std::cout << std::endl;
//     std::cout << "c "; for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
//     //std::cout << " var stack: " << active_variables << std::endl;
//     //std::cout << "c2 "; for(int lvl=0; lvl<level; ++lvl) std::cout << " ";
//     //std::cout << "con stack: " << active_constraints << std::endl
//       ;
// #endif


    
//     if(IS_OK(wiped_idx) && !active_constraints.empty()) {

// // #ifdef _DEBUG_AC
// //       std::cout << "\npropagate postponed constraints: " 
// // 		<< active_constraints << std::endl;
// // #endif

//       // propagate postponed constraint
//       culprit = active_constraints.select(constraints);

// #ifdef _DEBUG_AC
//       std::cout << "c "; for(int lvl=0; lvl<iteration; ++lvl) std::cout << " ";
//       std::cout << "  -propagate " << culprit << " (" ;

//       if(culprit.global()) {
// 	GlobalConstraint *gc = (GlobalConstraint*)(culprit.propagator);
		
// 	std::cout << gc->events << " ";
// 	std::cout.flush();
	
// 	std::cout << event2str(gc->event_type[gc->events[0]]) << " on " << gc->scope[gc->events[0]] ;
// 	for(unsigned int i=1; i<gc->events.size; ++i) {
// 	  std::cout << ", " << event2str(gc->event_type[gc->events[i]]) << " on " << gc->scope[gc->events[i]] ;
// 	}
//       }
//       std::cout << ") ";
//       Variable *scp = culprit.get_scope();
//       int arity = culprit.arity();
//       for(int i=0; i<arity; ++i)
// 	std::cout << scp[i].get_domain() << " ";
//       std::cout << "-> ";

// #endif

//       taboo_constraint = culprit.freeze();
//       wiped_idx = culprit.propagate(); 
//       taboo_constraint = culprit.defrost();

// #ifdef _DEBUG_AC
//       if(IS_OK(wiped_idx)) {
// 	Variable *scp = culprit.get_scope();
// 	int arity = culprit.arity();
// 	for(int i=0; i<arity; ++i)
// 	  std::cout << scp[i].get_domain() << " ";
// 	std::cout << "ok";
//       } else std::cout << "fail";
//       std::cout << std::endl;
// #endif	
      
//     } else if(active_variables.empty()) fix_point = true;
//   }
    
//   taboo_constraint = NULL;
//   active_constraints.clear();
//   if(!parameters.backjump) {
//     active_variables.clear();
//   }

// #ifdef _DEBUG_AC
//   if(!IS_OK(wiped_idx)) {
//     std::cout << "inconsistency found!" << std::endl;
//   } else {
//     std::cout << "done" << std::endl;
//   }
// #endif 
  
//   if(IS_OK(wiped_idx)) {
//     notify_success();   
//     return true;
//   } else {
//     ++statistics.num_failures;

//     //std::cout << "solver: notify failure" << std::endl;

//     notify_failure();
//     return false;
//   }
// }



// bool Mistral::Solver::propagate() 
// {

//   wiped_idx = -1;
//   culprit = NULL;

//   // if there is an objective function, it is propagated first, 
//   // since upon backtrack, it could fail even if n-1 variables are set
//   if(objective && objective->enforce())
//     wiped_idx = objective->objective.id();

//   ++statistics.num_filterings;

//   int *v_index;
//   Constraint **c_iter, **end, *c;
//   bool finished = active_constraints.empty() && active_variables.empty();
//   while( IS_OK(wiped_idx) && (!finished) ) {
  
//     while(!active_variables.empty()) {      
//       var = active_variables.pop( evt );
      
//       c_iter = constraint_graph_array[var]->first(EVENT_TYPE(evt));
//       v_index = constraint_graph_array[var]->get_index(c_iter);
//       end = constraint_graph_array[var]->first();

//       if(ASSIGNED(evt)) {
// 	while(c_iter < end) {
// 	  active_constraints.trigger(*c_iter, *v_index, evt);
//        	  c = (*c_iter)->notify_assignment(*v_index, level);
//        	  if(c) saved_cons.add(c);
// 	  ++c_iter;
// 	  ++v_index;
//       	}
//       	if(sequence.contain(variables[var])) {
//       	  sequence.remove(variables[var]);
//       	  assignment_level[var] = level;
//       	}
//       } else {
// 	//if(!(ASSIGNED(evt)))
// 	while(c_iter < end) {
//   	  active_constraints.trigger(*c_iter, *v_index, evt);
// 	  ++c_iter;
// 	  ++v_index;
// 	}
//       }
//     }

//     if(IS_OK(wiped_idx) && !active_constraints.empty()) {

//       culprit = active_constraints.select(constraints);

//       ++statistics.num_propagations;
      
//       wiped_idx = culprit->propagate();
      
//       culprit->defrost();
      
//      }

//   }
  
//   //  taboo_constraint = NULL;
//   active_constraints.clear();
//   active_variables.clear();
  
//   if(IS_OK(wiped_idx)) {
//     notify_success();
//   } else {
//     ++statistics.num_failures;
//     notify_failure();
//   }
//   return IS_OK(wiped_idx); //!wiped_out;
// }


#ifdef _MONITOR
void Mistral::Solver::monitor(Vector< Variable >& X) {
  for(unsigned int i=0; i<X.size; ++i)
    monitored.add(X[i].id());
  monitored_index.add(monitored.size);
}
void Mistral::Solver::monitor(Variable X) {
  monitored.add(X.id());
  monitored_index.add(monitored.size);
}
#endif


void Mistral::Solver::full_print() {
  for(int j=0; j<level; ++j) std::cout << "  ";
  std::cout << sequence << std::endl;
  for(unsigned int i=0; i<variables.size; ++i) {
    for(int j=0; j<level; ++j) std::cout << "  ";
    //variables[i].full_print();
    std::cout << std::endl;
  }
}

void Mistral::Solver::debug_print() {

//   std::cout << std::endl << this << std::endl;

// //   std::cout << "    currently changed: " << std::endl; 
// //   int k = changed_objs.size;
// //   while(k --> 0) {
// //     changed_objs[k]->debug_print();
// //     //std::cout << std::endl;
// //   }
// //   std::cout << std::endl;
//   int i, k;
//   for(i=var_trail_size.size-1; i>0; --i) {
//     std::cout << "    changed at level " << (i-1) << ": " << std::endl; 
//     k = var_trail_size[i];
//     while(k --> var_trail_size[i-1])
//       saved_vars[k]->debug_print();
//     //std::cout << std::endl;
//   }


//   std::cout << "    AC Queue: " << std::endl; 
// //   std::cout << active_constraints.active << std::endl;
// //   std::cout << active_constraints.triggers[0] << std::endl;
// //   std::cout << active_constraints.triggers[1] << std::endl;
// //   std::cout << active_constraints.triggers[2] << std::endl;

//   for(unsigned int i=0; i<3; ++i) {
//     if(!active_constraints.triggers[i].empty()) {
//       std::cout << "priority " << i << std::endl;
//       int elt = active_constraints.triggers[i].first();
//       while(elt != active_constraints.triggers[i]._head) {
// 	Constraint *cons = constraints[elt];
// 	std::cout << "\t" << cons << ": ";
// 	for(unsigned int j=0; j<cons->changes.size; ++j) {
// 	  int var = cons->changes[j];
// 	  int evt = cons->event_type[var];
// 	  std::cout << cons->scope[var] << "/" 
// 		    << (is_value(evt) ? "v" : (is_range(evt) ? "r" : "d") )
// 		    << (is_upper_bound(evt) ? "u" : "")
// 		    << (is_lower_bound(evt) ? "l" : "")
// 		    << " ";
// 	}
// 	std::cout << std::endl;
// 	elt = active_constraints.triggers[i].next[elt];
//       } 
//     }
//   }

// //   for(unsigned int i=0; i<active_constraints.size; ++i) {
// //     Constraint *cons = constraints[active_constraints[i]];
// //     std::cout << cons << ": ";
// //     for(unsigned int j=0; j<cons->changes.size; ++j) {
// //       int var = cons->changes[j];
// //       int evt = cons->event_type[var];
// //       std::cout << cons->scope[var] << "/" 
// // 		<< (is_value(evt) ? "v" : (is_range(evt) ? "r" : "d") )
// // 		<< (is_upper_bound(evt) ? "u" : "")
// // 		<< (is_lower_bound(evt) ? "l" : "")
// // 		<< " ";
// //     }
// //     std::cout << std::endl;
// //   }
}

// std::string Mistral::Solver::getString() const {
//   std::string return_str = "Variables:\n";
//   for(unsigned int i=0; i<variables.size; ++i)
//     return_str += ("\t"+toString(variables[i])+" in "+toString(variables[i]->domain)+"\n");

//   return_str += "\nConstraints:\n";
//   for(unsigned int i=0; i<constraints.size; ++i)
//     return_str += ("\t"+toString(constraints[i])+"\n");

//   return_str += ("\nSearch on "+toString(search.sequence)+"\n");

//   return return_str;
// }

void Mistral::Solver::print_clist(int k) {
  for(Event trig = 0; trig<3; ++trig) 
     for(int cons = constraint_graph[k].on[trig].size; --cons>=0;) 
       std::cout << "[" << constraint_graph[k].on[trig][cons].id() << "]";
  std::cout << "\n";
}

std::ostream& Mistral::Solver::display(std::ostream& os, const int current) {

  if(objective) os << objective << std::endl;

  os << "Variables:\n";


  Variable *scope;
  int arity;

  Vector<Variable> rem_vars;
  for(unsigned int i=0; i<variables.size; ++i) {
    if(!(domain_types[i] & REMOVED_VAR) 
       && (current != 1 || sequence.contain(i))
       && (current != 2 || !(variables[i].is_ground()))) {

      os << "  " << variables[i] << " in " << variables[i].get_domain() ; //<< "\n";
    
      os << ": " ;
      for(Event trig = 0; trig<3; ++trig) 
	for(int cons = constraint_graph[i].on[trig].size; --cons>=0;) {
	  if(current) {
	    os << "[" << constraint_graph[i].on[trig][cons].id() 
	       << constraint_graph[i].on[trig][cons].symbol(); 
	    
	    scope = constraint_graph[i].on[trig][cons].get_scope();
	    arity = constraint_graph[i].on[trig][cons].arity();
	    
	    int k=0, j=0;
	    for(; j<arity && k<1; ++j) {
	      if(!scope[j].is_ground()) {
		++k;
		std::cout << scope[j].id() << " " ;
	      }
	    }
	    for(; j<arity && k<2; ++j) {
	      if(!scope[j].is_ground()) {
		++k;
		std::cout << scope[j].id() ; //<< " " ;
	      }
	    }
	    
	    os
	      //<< constraint_graph[i].on[trig][cons].symbol() 
	      //constraint_graph[i].on[trig][cons].id() 
	      << "]";
	  } else {
	    os << "[" << 
	      //constraint_graph[i].on[trig][cons].symbol() 
	      constraint_graph[i].on[trig][cons].id() 
	       << "]";
	  }
	}
      os << "\n";
    
    } else {

      rem_vars.add(variables[i]);

    }
  }



  if(!current) {  
  if(!rem_vars.empty()) {
    os << "  (suppressed:" ;
    for(unsigned int i=0; i<rem_vars.size; ++i)
      os << " " << rem_vars[i];
    os << ")";
  }

  os << "\nConstraints:\n" ; //<< posted_constraints << std::endl;
  for(unsigned int i=0; i<posted_constraints.size; ++i) {
    
    //os << posted_constraints[i] << std::endl;

    os << "  [" << constraints[posted_constraints[i]].id() << "]: " << constraints[posted_constraints[i]] << "\n";
  }
  }

  return os;
}



//Mistral::Decision 
void Mistral::Solver::learn_nogood() {

#ifdef _DEBUG_NOGOOD
#ifdef _DEBUG_SEARCH
  if(_DEBUG_SEARCH) {
    for(int i=0; i<level; ++i) std::cout << " "; 
    std::cout << "conflict: " ; 
    //std::cout << (int*)base << std::endl;
    //std::cout << (int*)(base->conflict) << std::endl ;
    print_clause(std::cout, base->conflict);
    std::cout << std::endl;
  }
#endif

  //#ifdef _DEBUG_NOGOOD

  // std::cout << base->changes << std::endl
  // 	    << base->events << std::endl;

  std::cout << active_variables << std::endl;


  for(unsigned int i=num_search_variables-1; i>=sequence.size; --i) {
    std::cout << num_search_variables-1-i << "\t"
	      << assignment_level[sequence[i].id()] << "\t"
	      << sequence[i] << " == " << sequence[i].get_min() << "\t";
    if(reason[sequence[i].id()]) {
      print_clause(std::cout, reason[sequence[i].id()]);
    } else {
      std::cout << "decision";
    }
    std::cout << std::endl;
  }
#endif

  //if(base->conflict) {



  Triplet < int, Event, ConstraintImplementation* > var_evt;
  while(!active_variables.empty()) {
    var_evt = active_variables.pop_front();
    if(ASSIGNED(var_evt.second) && sequence.contain(variables[var_evt.first])) {
      sequence.remove(variables[var_evt.first]);
      assignment_level[var_evt.first] = level;
    }
  }


  unsigned int j;
  int pathC = 0, index = sequence.size-1;//, index = assumptions.size;
  Lit p=0, q;
  Atom     a;
  Variable x;
  int lvl;
  Clause *current_clause = base->conflict;
  double *lit_activity = base->lit_activity.stack_;
  double *var_activity = base->var_activity.stack_;


  backtrack_level = 0;
  //std::cout << current_clause->size << std::endl;

  
  learnt_clause.clear();
  learnt_clause.add(p);
  //visited.clear();

  // for(int i=num_search_variables-1; i>=sequence.size; --i) {
  //   std::cout << " " << sequence[i];
  // }

  do {
    // add the parents of the conflict to the current set of visited atoms

    // if(!current_clause) {
    //   std::cout << "No conflict!!" << std::endl;
    //   exit(1);
    // }


    Clause& con = *(current_clause);


    //std::cout << con.size << std::endl;

#ifdef _DEBUG_NOGOOD
    print_clause( std::cout, current_clause );
    std::cout << std::endl;
#endif
    for(j=0; j<con.size; ++j) {
      q = con[j];
      a = UNSIGNED(q);
      x = variables[a];
      lvl = assignment_level[a];
      //lvl = LEVEL(state[a]);
#ifdef _DEBUG_NOGOOD
      std::cout << "\t" ;
      print_literal(std::cout, q); 
      std::cout << ": ";
#endif
      if( !visited.fast_contain(a) ) {
	lit_activity[q] += parameters.activity_increment;
	var_activity[a] += parameters.activity_increment;
	visited.fast_add(a);
	// we'll need to replace 'a' by its parents since its level is too high
	//if(lvl >= decisions.back()) {
	if(lvl >= level) {
#ifdef _DEBUG_NOGOOD
	  std::cout << "expend" << std::endl;
#endif
	  ++pathC;
	} else {
	  // q's level is below the current level, we are not expending it further
	  learnt_clause.add(q);
#ifdef _DEBUG_NOGOOD
	  std::cout << "add to the clause" ;
	  for(unsigned int k=0; k<learnt_clause.size; ++k) {
	    std::cout << " ";
	    print_literal(std::cout, learnt_clause[k]);
	  }
	  std::cout << std::endl;
#endif
	  if(lvl > backtrack_level)
	    backtrack_level = lvl;
	}
      }
#ifdef _DEBUG_NOGOOD
      else {
	std::cout << "visited" << std::endl;
      }
#endif
    }
    // jump to the next visited atom that need be further expended
    //if(index<variables.size-1) {
    while(!visited.fast_contain(sequence[++index].id()))//  {
    //   if(index==variables.size-1) { 
    // 	  pathC = 1;
    // 	  break;
    // 	}
    //   }
    // }
      ;
    x = sequence[index];
    a = x.id();
    p = ((2*a) | (x.get_min()));
    lvl = assignment_level[a];
    //p = (2*a) | SIGN(state[a]); //polarity[a];
    //lvl = LEVEL(state[a]);

#ifdef _DEBUG_NOGOOD
    std::cout << "explore ";
    print_literal(std::cout, p); 
    std::cout << " ";
    //std::cout.flush();
#endif

    if( pathC > 1 ) {
      // there are still atoms to expend, we start with 'a'
      current_clause = reason[a];
      visited.fast_add(a);
    } 
#ifdef _DEBUG_NOGOOD
    else {
      std::cout << std::endl;
    }
#endif

  } while( --pathC );
  // p is the last decision, since all atoms above it in the
  // assumption stack have been skipped or expended.
  learnt_clause[0] = NOT(p);    

#ifdef _DEBUG_SEARCH
  if(_DEBUG_SEARCH) {
    std::cout << " (";
    for(unsigned int i=0; i<learnt_clause.size; ++i) {
      std::cout << " " ;//<< learnt_clause[i];
      print_literal(std::cout, learnt_clause[i]);
    }
    std::cout << " )" << std::endl;
  }
#endif
        
  //exit(1);




  if( learnt_clause.size != 1 ) {
    base->learn(learnt_clause, (parameters.init_activity ? parameters.activity_increment : 0.0));
    //add_clause( learnt, learnt_clause, stats.learnt_avg_size );
    reason[UNSIGNED(p)] = base->learnt.back();
  }
  visited.clear();


  //backjump_decision = decision(variables[UNSIGNED(p)], Decision::REMOVAL, SIGN(p));

#ifdef _DEBUG_NOGOOD
  std::cout << "backtrackLevel = " << backtrack_level << "/" << (decisions.size) << std::endl;
#endif

//   while(level>backtrack_level) {
//     restore();
//     decisions.pop();
//   }

  //return decision;

}

void Mistral::Solver::forget() {
  if(base) base->forget(parameters.forgetfulness);
}


void Mistral::Solver::branch_right() {
  Mistral::Decision deduction;
  //backtrack_level = level-1;
  // if(parameters.backjump) {
  //   //int backtrack_level=level-1;
  //   decision = learn_nogood_and_backjump(); //backtrack_level);
  //   else {
  //     while(level>backtrack_level) {
  // 	restore();
  // 	decisions.pop();
  //     }
  //   }
  // } else {


  if(parameters.backjump) {
    //decisions.size += (backtrack_level-level);
    Lit p = learnt_clause[0];
    deduction = Decision(variables[UNSIGNED(p)], Decision::REMOVAL, NOT(SIGN(p)));
  } else {
    backtrack_level = level-1;
    //deduction = decisions.pop();
    deduction = decisions.back();
    deduction.invert();
  }

  //decisions.size = backtrack_level;

  // if(backtrack_level == level)
  //   {
  //     std::cout << "CA ALORS!" << std::endl;
  //   }

  restore(backtrack_level);  

  //decision = decisions.pop(); 
  //decision.invert();
  //}

  // if( limits_expired() ) status = LIMITOUT;
  // else {
  
#ifdef _DEBUG_SEARCH
  if(_DEBUG_SEARCH) {
    std::cout << "c";
    for(unsigned int k=0; k<=decisions.size; ++k) std::cout << " ";
    std::cout << deduction << std::endl;
  }
#endif
  
  //decisions.back(-1).make();
  //decision.make();
  deduction.make();
  //}



}


void Mistral::Solver::backjump() {
  int backtrack_level = culprit.get_backtrack_level();
  decisions.size -= (level - backtrack_level);
  restore(backtrack_level);
  Decision decision = culprit.get_decision();
  
#ifdef _DEBUG_SEARCH
  if(_DEBUG_SEARCH) {
    std::cout << "c";
    for(unsigned int k=0; k<=decisions.size; ++k) std::cout << " ";
    std::cout << decision << std::endl;
  }
#endif

  decision.make();
}

void Mistral::Solver::branch_left() {


  save();

  //heuristic->check_consistency();
  
  // ((GenericHeuristic< 
  //   GenericWeightedDVO< 
  //     FailureCountManager, 
  //     //PruningCountManager, 
  //     MinDomainOverWeight >,
  //   //GenericDVO< MinDomainOverDegree >,			    
  //     MinValue >*)heuristic)->var.manager->check_consistency();

  //heuristic->display(std::cout);

  Mistral::Decision decision = heuristic->branch();


  // std::cout << variables[95] << " == 0 " << ((GenericHeuristic< GenericWeightedDVO< LiteralActivityManager, MaxWeight >, BoolMinWeightValue >*)heuristic)->var.manager->lit_activity[190] << std::endl;




  // std::cout << variables[95] << " == 1 " << ((GenericHeuristic< GenericWeightedDVO< LiteralActivityManager, MaxWeight >, BoolMinWeightValue >*)heuristic)->var.manager->lit_activity[NOT(190)] << std::endl;


  // std::cout << decision << " " << decision.var.get_domain() << std::endl;
  
  //if(decision.var.is_ground()) exit(1);


  // if(decisions.size && (decisions.back() == decision)) {
  //   exit(1);
  // }

  reason[decision.var.id()] = NULL;
  decisions.add(decision);

#ifdef _DEBUG_SEARCH
  if(_DEBUG_SEARCH) {
    std::cout << "c";
    for(unsigned int k=0; k<=decisions.size; ++k) std::cout << " ";
    std::cout << decision << std::endl;
  }
#endif

#ifdef _SAFE
  if(decision.var.is_ground()) {

    std::cerr << "The variable " << decision.var 
	      << " is ground but still in the sequence!! (abort)" << std::endl;

    exit(1);
  }
#endif

  //if(decision.var.id() == 16) exit(1);

  //std::cout << 11 << std::endl;

  decision.make();

  //std::cout << 21 << std::endl;

  notify_decision();

  //std::cout << 31 << std::endl;

  // //if(level>=2)
  // std::cout << "X4's trail: " << (variables[4].domain_type == RANGE_VAR ?
  // 				  ((VariableRange*)(variables[4].variable))->trail_ :
  // 				  ((VariableBitmap*)(variables[4].variable))->trail_)
  // 	    << " " << variables[4].get_domain() << std::endl;
}


 Mistral::Outcome Mistral::Solver::satisfied() {    
#ifdef _DEBUG_SEARCH
   if(_DEBUG_SEARCH) {
     std::cout << " c";
     for(unsigned int k=0; k<=decisions.size; ++k) std::cout << " ";
     std::cout << " SAT!" << std::endl; 
   }
#endif

  //std::cout << this << std::endl;

  unsigned int i, j, k;


  if(parameters.checked) {

    //std::cout << posted_constraints << std::endl;

    /// check the current solution
    Vector< int > tmp_sol;
    Variable *scope;
    Constraint C;
    bool all_assigned;


    for(i=0; i<posted_constraints.size; ++i) {


      //std::cout << posted_constraints[i] << " / " << constraints.size << std::endl;

      all_assigned = true;
      C = constraints[posted_constraints[i]];
      //C.consolidate();

      k=C.arity();
      scope = C.get_scope();
      for(j=0; j<k; ++j) {
	if(scope[j].is_ground()) 
	  tmp_sol.add(scope[j].get_value());
	else {
	  tmp_sol.add(scope[j].get_min());
	  all_assigned = false;
	  //break;
	}
      }

      bool consistent = true;

      // if(C.id() == 48996) {
      // 	std::cout << "check " << C << "(variables are" << (all_assigned ? " " : " not ") << "all assigned)" << std::endl;
      // }

      if(!all_assigned) {


	/// !!! This checks that all values are AC, this is too strong
	// for(j=0; j<k && consistent; ++j) {
	//   if(!scope[j].is_ground()) {
	//     int vali, vnext = scope[j].get_min();
	//     do {
	//       vali = vnext;
	//       if(!C.find_support(j, vali)) consistent = false;
	//       vnext = scope[j].next(vali);
	//     } while( consistent && vali<vnext );
	//   }
	// }


	/// This checks that all bounds are BC
	for(j=0; j<k && consistent; ++j) {
	  if(!scope[j].is_ground()) {
	    if(!C.find_bound_support(j, scope[j].get_min())) consistent = false;
	    else if(!C.find_bound_support(j, scope[j].get_max())) consistent = false;
	  }
	}
	

      } else {
	consistent = !C.check(tmp_sol.stack_);
      }

      if(!consistent)
      {
	
	if(tmp_sol.size < k) {
	  std::cerr << "\nError: solution does not satisfy c" << C.id() << ": " << C << tmp_sol << " (backtracking)"<< std::endl;
	  exit(0);
	} else {
	  std::cerr << "\nError: solution does not satisfy c" << C.id() << ": " << C ;
	  for(j=0; j<k; ++j) {
	    std::cerr << " " << scope[j].get_domain();
	  }
	  std::cerr << " (backtracking)"<< std::endl;
	  exit(0);
	}
	if( decisions.empty() ) return UNSAT;
	else if( limits_expired() ) return LIMITOUT;
	else {
	  branch_right();
	  return UNKNOWN;
	}
      }
      tmp_sol.clear();
    }
  }

  /// store the solution 
  for(i=0; i<variables.size; ++i) {
    last_solution_lb[i] = variables[i].get_min();
    last_solution_ub[i] = variables[i].get_max();

    //std::cout << variables[i] << " := " << last_solution_lb[i] << " ";

  }
  //std::cout << std::endl;
  ++statistics.num_solutions;

  /// notify the objective and return the outcome
  Outcome result = //(_exit_on_solution_ ? SAT : objective->notify_solution(this));
    objective->notify_solution(this);

  statistics.objective_value = objective->value();

  for(i=0; i<solution_triggers.size; ++i) {
    solution_triggers[i]->notify_solution();
  }
  
  return result;
  
  //return SAT;
}


Mistral::Outcome Mistral::Solver::exhausted() {    
#ifdef _DEBUG_SEARCH
  if(_DEBUG_SEARCH) {
    std::cout << " c UNSAT!" << std::endl; 
  }
#endif
  
  Outcome value = UNSAT;
  if(statistics.num_solutions) value = objective->notify_exhausted();
  return value;
}

 bool Mistral::Solver::limits_expired() {
  
  return (parameters.limit && 
	  ((parameters.time_limit > 0.0 && (get_run_time() - statistics.start_time) > parameters.time_limit) ||
	   (parameters.node_limit > 0 && (statistics.num_nodes > parameters.node_limit)) ||
	   (parameters.fail_limit > 0 && (statistics.num_failures > parameters.fail_limit)) ||
	   (parameters.restart_limit > 0 && (statistics.num_failures > parameters.restart_limit)) ||
	   (parameters.propagation_limit > 0 && (statistics.num_propagations > parameters.propagation_limit)) // ||
	   // (parameters.backtrack_limit > 0 && (statistics.num_backtracks > parameters.backtrack_limit))
	   ));
}

// void Mistral::Search::init_search(Vector< Variable >& seq, VarOrdering *h, RestartPolicy *p) {
//   for(unsigned int i=0; i<seq.size; ++i) {
//     std::cout << "insert " << seq[i] << std::endl;
//     sequence.insert(seq[i]);    
//   }
//   if(heuristic) delete heuristic;
//   if(policy) delete policy;
//   heuristic = (h ? h : new NoOrder(sequence));
//   policy = p;
// }


// std::string Mistral::toString(const Mistral::Solver& x) {
//   return x.getString();
// }



/**
   Searches for an assignment of the variables in sequences 

   Stops when the objecive is satisfied, or when the search tree is exhausted 
   - SATISFACTION OBJECTIVE: returns 'SAT' when a solution is found, 'UNSAT' if the search tree is exhausted, and 'UNKNOWN' otherwise
   - OPTIMIZATION OBJECTIVE: returns 'OPT' when the search tree is exhausted and 'UNKNOWN' otherwise
   - ENUMERATION  OBJECTIVE: returns 'ALL' when the search tree is exhausted and 'UNKNOWN' otherwise [TODO]
   
          The argument 'root' controls how deep we can backtrack. i.e., the root of the search tree.
          It is possible to start the search with the decision stack and trail non empty, and backtrack
	  on them (if 'root' is set to something lower than their size), or not.
 */
Mistral::Outcome Mistral::Solver::chronological_dfs(const int root) 
{

  //std::cout << "start search rooted at level " << root << std::endl;

  int status = UNKNOWN;
  while(status == UNKNOWN) {

    // if(decisions.size != level) {
    //   std::cout << "decisions.size/level = " << decisions.size << "/" << level << std::endl;
    //   exit(1);
    // }

    if(propagate()) {

#ifdef _MONITOR
    monitor_list.display(std::cout);
    std::cout << std::endl;
    //display(std::cout, 2);
    check_constraint_graph_integrity();
#endif
            
      ++statistics.num_nodes;
      if( sequence.empty()  ) //status = satisfied(_exit_on_solution_);
	status = satisfied();//objective);
      else branch_left();

    } else {

      if( parameters.backjump ) learn_nogood();
      //if( decisions.empty() ) status = exhausted(); //objective);
      if( level == root ) status = exhausted(); //objective);
      else if( limits_expired() ) status = LIMITOUT;
      else branch_right();

    }

  }
  return status;
}


Mistral::Outcome Mistral::Solver::conflict_directed_backjump()
{
  int status = UNKNOWN;
  while(status == UNKNOWN) {
    if(propagate()) {
      if( sequence.empty() ) status = satisfied();
      else branch_left();
    } else {
      if( decisions.empty() ) status = UNSAT;
      else if( limits_expired() ) status = LIMITOUT;
      else backjump();
    }
  }

  return status;
}

double *Mistral::Solver::get_literal_activity() {
  return base->lit_activity.stack_;
}

std::ostream& Mistral::operator<< (std::ostream& os, Mistral::Solution& x) {
  return x.display(os);
}
std::ostream& Mistral::operator<< (std::ostream& os, Mistral::Solution* x) {
  return x->display(os);
}

std::ostream& Mistral::operator<< (std::ostream& os, Mistral::Solver& x) {
  return x.display(os);
}
std::ostream& Mistral::operator<< (std::ostream& os, Mistral::Solver* x) {
  return x->display(os);
}

std::ostream& Mistral::operator<< (std::ostream& os, Mistral::ConstraintQueue& x) {
  return x.display(os);
}
std::ostream& Mistral::operator<< (std::ostream& os, Mistral::ConstraintQueue* x) {
  return x->display(os);
}

std::ostream& Mistral::operator<< (std::ostream& os, const Mistral::SolverStatistics& x) {
  return x.display(os);
}
std::ostream& Mistral::operator<< (std::ostream& os, const Mistral::SolverStatistics* x) {
  return x->display(os);
}


void Mistral::SearchMonitor::add(Variable x) {
  sequence.add(0);
  sequence.add(x.id());
}
void Mistral::SearchMonitor::add(Constraint x) {
  sequence.add(1);
  sequence.add(x.id());
}
// void Mistral::SearchMonitor::add(std::string& x) {
//   sequence.add(2);
//   sequence.add(strs.size);
//   strs.push_back(x);
// }
void Mistral::SearchMonitor::add(const char* x) {
  sequence.add(2);
  sequence.add(strs.size());
  strs.push_back(x);
}

std::ostream& Mistral::SearchMonitor::display( std::ostream& os ) const {
  for(unsigned int i=0; i<sequence.size; i+=2) {
    if(sequence[i] == 0) {
      os << solver->variables[sequence[i+1]].get_domain();
    } else if(sequence[i] == 1) {
      os << solver->constraints[sequence[i+1]];
    } else {
      os << strs[sequence[i+1]];
    }
  }
  return os;
}

Mistral::SearchMonitor& Mistral::operator<< (Mistral::SearchMonitor& os, VarArray& x) {
  os.add("(");
  os.add(x[0]);
  for(unsigned int i=1; i<x.size; ++i) {
    os.add(" ");
    os.add(x[i]);
  }
  os.add(")");
  return os;
}
Mistral::SearchMonitor& Mistral::operator<< (Mistral::SearchMonitor& os, Variable& x) {
  os.add(x);
  return os;
}
Mistral::SearchMonitor& Mistral::operator<< (Mistral::SearchMonitor& os, Constraint& x) {
  os.add(x);
  return os;
}
Mistral::SearchMonitor& Mistral::operator<< (Mistral::SearchMonitor& os, const char* x) {
  os.add(x);
  return os;
}
// Mistral::SearchMonitor& Mistral::operator<< (Mistral::SearchMonitor& os, const char* x) {
//   std::string s(x);
//   os.add(s);
//   return os;
// }
Mistral::SearchMonitor& Mistral::operator<< (Mistral::SearchMonitor& os, const int x) {
  //std::ostringstream o_aux;
  //o_aux << x;
  //const char *y = ;
  // os.add((o_aux.str().c_str()));
  //o_aux.close();


  //std::string number = boost::lexical_cast(x);
  // char buf[255];
  // sprintf(buf,"%d",x);


  os << " " ; //int2str(x).c_str() ;

  //os << number.c_str() ;
  return os;
}



// std::ostream& Mistral::operator<< (std::ostream& os, Mistral::ConstraintArray& x) {
//   return x.display(os, true);
// }
// std::ostream& Mistral::operator<< (std::ostream& os, Mistral::ConstraintArray* x) {
//   return x->display(os, true);
// }



// Mistral::ConstraintTriggerArray::ConstraintTriggerArray() {
// }

// Mistral::ConstraintTriggerArray::ConstraintTriggerArray(const int size) {
//   initialise(size);
// }

// void Mistral::ConstraintTriggerArray::initialise(const int size) {
//   for(int i=0; i<3; ++i)
//     on[i].initialise(size);

//   /*
//   value_trigger.initialise(size);
//   range_trigger.initialise(size);
//   domain_trigger.initialise(size);
//   */
// }

// Mistral::ConstraintTriggerArray::~ConstraintTriggerArray() { }

// std::ostream& Mistral::ConstraintTriggerArray::display(std::ostream& os, bool full) const {
//   return os;
// }



// Mistral::ConstraintArray::ConstraintArray() {
//   bound[0] = bound[1] = data = end = trigger[0] = trigger[1] = trigger[2] = trigger[3] = NULL;
//   var_index = NULL;
// }

// Mistral::ConstraintArray::ConstraintArray(const int size) {
//   initialise(size);
// }

// void Mistral::ConstraintArray::initialise(const int size) {
//   data = new Constraint*[size];
//   end = data+size;
//   bound[0] = bound[1] = trigger[0] = trigger[1] = trigger[2] = trigger[3] = data+(size/2);
//   var_index = new int[size];
// }

// Mistral::ConstraintArray::~ConstraintArray() { delete [] data; }

// void Mistral::ConstraintArray::extend() {

//   //std::cout << "\nextend!! " << std::endl;

//   int size = (end-data);
//   Constraint **new_data = new Constraint *[2*size];
//   int *new_var_index = new int[2*size];

//   int old_start_offset = (trigger[0]-data);
//   int old_end_offset = (end-trigger[3]);

//   int offset = (size + old_end_offset + old_start_offset)/2;
      
//   memcpy(new_data+offset, trigger[0], 
// 	 sizeof(Constraint*)*(trigger[3]-trigger[0]));
//   memcpy(new_var_index+offset, var_index+(trigger[0]-data), 
// 	 sizeof(int)*(trigger[3]-trigger[0]));

//   end = new_data+2*size;
//   bound[1] = new_data+offset+(bound[1]-trigger[0]);
//   bound[0] = new_data+offset+(bound[0]-trigger[0]);

//   trigger[1] = new_data+offset+(trigger[1]-trigger[0]);
//   trigger[2] = new_data+offset+(trigger[2]-trigger[0]);
//   trigger[3] = new_data+offset+(trigger[3]-trigger[0]);
//   trigger[0] = new_data+offset;      

//   for(Constraint** cit = trigger[0]; cit<trigger[3]; ++cit) {
//     (*cit)->index[new_var_index[cit-new_data]] = cit;
//   }

//   delete [] data;
//   delete [] var_index;

//   data = new_data;
//   var_index = new_var_index;



//   //check_integrity();

// }

// void Mistral::ConstraintArray::remove(Mistral::Constraint** c_index) {
//   Constraint *d;
//   int d_rank;

//   if(c_index < trigger[1]) {
//     // value trigger, we move *trigger[0] to c_index and increment trigger[0]

//     d = *(trigger[0]);
//     *c_index = d;
//     d_rank = var_index[c_index-data] = var_index[trigger[0]-data];
//     d->index[d_rank] = c_index;
//     ++trigger[0];

//   } else if(c_index >= trigger[2]) {
//     // domain trigger, we move *(trigger[3]-1) to c_index and decrement trigger[3]

//     d = *(--trigger[3]);
//     *c_index = d;
//     d_rank = var_index[c_index-data] = var_index[trigger[3]-data];
//     d->index[d_rank] = c_index;

//   } else {
//     // remove in the left: put *trigger[0] at *trigger[1] and *trigger[1] at c_index
//     // remove in the right: put *trigger[3]-1 at *trigger[2]-1 and *trigger[2]-1 at c_index and decrement trigger[3] and trigger[2]

//     d = *(--trigger[2]);
//     *c_index = d;
//     d_rank = var_index[c_index-data] = var_index[trigger[2]-data];
//     d->index[d_rank] = c_index;

//     d = *(--trigger[3]);
//     if(trigger[3] != trigger[2]) {
//       *trigger[2] = d;
//       d_rank = var_index[trigger[2]-data] = var_index[trigger[3]-data];
//       d->index[d_rank] = trigger[2];
//     }
//   }
// }



// // given a constraint and a constraint list, get the index of the constraint in the list
// // 


// Mistral::Constraint** Mistral::ConstraintArray::add_range(Mistral::Constraint *c, const int idx) {
//   // add constraint c to its ith variable. Here we assume that 
//   //  1/ c has been declared and removed, 
//   //  2/ all removes of range triggers have been made by the right

//   if(trigger[2] < trigger[3]) {	

//     int d_rank = var_index[trigger[2]-data];
//     var_index[trigger[3]-data] = d_rank;
//     var_index[trigger[2]-data] = idx;
	
//     *trigger[3] = *trigger[2];
//     *trigger[2] = c;
	
//     (*trigger[3])->index[d_rank] = trigger[3];

//   } else {


//     //std::cout << "here " << (trigger[2]-data) << std::endl;

//     var_index[trigger[2]-data] = idx;
//     *trigger[2] = c;

//   }

//   ++trigger[3];
      
//   return trigger[2]++;
// }


// Mistral::Constraint** Mistral::ConstraintArray::add_range_before(Mistral::Constraint *c, const int idx) {
//   // put trigger[1]-1 at trigger[0]-1 put c at trigger[1]-1 and decrement them; 
	
//   if(trigger[0] < trigger[1]) {

//     int d_rank = var_index[--trigger[1]-data];
//     var_index[--trigger[0]-data] = d_rank;
//     var_index[trigger[1]-data] = idx;
       
//     *trigger[0] = *trigger[1];
//     *trigger[1] = c;
       
//     (*trigger[0])->index[d_rank] = trigger[0];

//   } else {

//     var_index[--trigger[1]-data] = idx;
//     *trigger[1] = c;
//     --trigger[0];

//   }
       
     
//   return trigger[1];
// }

// Mistral::Constraint** Mistral::ConstraintArray::add_value(Mistral::Constraint *c, const int idx) {
//   // add constraint c to its ith variable. Here we assume that 
//   //  1/ c has been declared and removed, 
//   //  2/ all removes of range triggers have been made by the right
	
//   *(--trigger[0]) = c;
//   var_index[trigger[0]-data] = idx;
      
//   return trigger[0];
// }


// Mistral::Constraint** Mistral::ConstraintArray::add_domain(Mistral::Constraint *c, const int idx) {
//   // add constraint c to its ith variable. Here we assume that 
//   //  1/ c has been declared and removed, 
//   //  2/ all removes of range triggers have been made by the right
	
//   var_index[trigger[3]-data] = idx;
//   *trigger[3] = c;
      
//   return trigger[3]++;
// }

// Mistral::Constraint** Mistral::ConstraintArray::declare_range(Mistral::Constraint *c, const int idx) {
//   Constraint **ptr;
//   if(bound[1] < end) {
//     ++bound[1];
//     ptr = add_range(c, idx);
//   } else if(bound[0] > data) {
//     --bound[0];
//     ptr = add_range_before(c, idx);
//   } else {
//     extend();
//     --bound[0];
//     ptr = add_range_before(c, idx);
//   }
//   return ptr;
// }
	  
// Mistral::Constraint** Mistral::ConstraintArray::declare_value(Mistral::Constraint *c, const int idx) {
//   if(bound[0] == data) extend();
//   --bound[0];
//   return add_value(c, idx);	
// }


// Mistral::Constraint** Mistral::ConstraintArray::declare_domain(Mistral::Constraint *c, const int idx) {
//   if(bound[1] == end) extend();
//   ++bound[1];
//   return add_domain(c, idx);	
// }


// std::ostream& Mistral::ConstraintArray::display(std::ostream& os, bool full) const {
//   if(full) {
//     os << (end-data) << " [" << (bound[0]-data) << "][" << (trigger[0]-bound[0]) << "]";
//   }
//   os << "(";

//   BitSet all_ids(0, 1000, BitSet::empt);
//   bool bug = false;

//   for(int i=0; i<3; ++i) {
//     if(full) os << i << " " << (trigger[i+1]-trigger[i]) << ": ";
//     for(Constraint **cit = trigger[i]; cit<trigger[i+1]; ++cit) {
//       if(cit>trigger[i]) os << ", ";
//       os << (*cit)->id ;

//       if(all_ids.contain((*cit)->id)) {
// 	bug = true;
//       }
      
//       all_ids.add((*cit)->id);
//     }
//     if(i<2) os << "|";
//   }

//   os << ")";
//   if(full) {
//     os << "[" << (bound[1]-trigger[3]) << "][" << (end-bound[1]) << "]";
//   }
  
//   if(bug) {
//     os << std::endl;
//     exit(1);
//   }

//   return os;
// }

// void Mistral::ConstraintArray::check_integrity() const {

//   //std::cout << "[" << (trigger[0]-data) << ".." << (trigger[3]-data) << "]" << std::endl;

//   for(Constraint **cit = trigger[0]; cit<trigger[3]; ++cit) {
//     if((*cit)->index[var_index[cit-data]] != cit) {
//       std::cout << " on " ;
//       display(std::cout, true);
//       std::cout << ":\n  "
// 		<< "problem with " << (*cit)->id << " at rank " << (cit-data) 
// 		<< ": " << cit << " v " << (*cit)->index[var_index[cit-data]] 
// 		<< " " << (*cit) << " " << (*cit)->id << std::endl;
//       exit(1);
//     }
//   }
// }


/*
Mistral::ValueTrigger::ValueTrigger(ConstraintArray * a) {
  array = a;

  bound = array->bound[0];
  data = array->data;
  trigger = array->trigger[0];
  var_index = array->var_index;
}

Mistral::ValueTrigger::~ValueTrigger() { 
  bound = data = trigger = NULL;
  var_index = NULL;
}

void Mistral::ValueTrigger::remove(Mistral::Constraint** c_index) {
  Constraint *d;
  int d_rank;

  d = *(trigger);
  *c_index = d;
  d_rank = var_index[c_index-data] = var_index[trigger-data];
  d->index[d_rank] = c_index;
  ++trigger;
}

Mistral::Constraint** Mistral::ValueTrigger::add(Mistral::Constraint *c, const int idx) {
  // add constraint c to its ith variable. Here we assume that 
  //  1/ c has been declared and removed, 
  //  2/ all removes of range triggers have been made by the right
	
  *(--trigger) = c;
  var_index[trigger-data] = idx;
      
  return trigger;
}
	  
Mistral::Constraint** Mistral::ValueTrigger::declare(Mistral::Constraint *c, const int idx) {
  if(bound == data) array->extend();
  --bound;
  return add(c, idx);	
}




Mistral::RangeTrigger::RangeTrigger(ConstraintArray * a) {
  array = a;

  bound[0] = array->bound[0];
  bound[1] = array->bound[1];
  data = array->data;
  trigger[0] = array->trigger[2];
  trigger[1] = array->trigger[3];
  var_index = array->var_index;
}

Mistral::RangeTrigger::~RangeTrigger() { 
  bound[0] = bound[1] = data = trigger[0] = trigger[1] = NULL;
  var_index = NULL;
}

void Mistral::RangeTrigger::remove(Mistral::Constraint** c_index) {
  Constraint *d;
  int d_rank;

  // remove in the left: put *trigger[0] at *trigger[1] and *trigger[1] at c_index
  // remove in the right: put *trigger[3]-1 at *trigger[2]-1 and *trigger[2]-1 at c_index and decrement trigger[3] and trigger[2]
  
  d = *(--trigger[0]);
  *c_index = d;
  d_rank = var_index[c_index-data] = var_index[trigger[0]-data];
  d->index[d_rank] = c_index;
  
  d = *(--trigger[1]);
  if(trigger[1] != trigger[0]) {
    *trigger[0] = d;
    d_rank = var_index[trigger[0]-data] = var_index[trigger[1]-data];
    d->index[d_rank] = trigger[0];
  }
  
}

Mistral::Constraint** Mistral::RangeTrigger::add(Mistral::Constraint *c, const int idx) {

 // add constraint c to its ith variable. Here we assume that 
  //  1/ c has been declared and removed, 
  //  2/ all removes of range triggers have been made by the right

  if(trigger[0] < trigger[1]) {	

    int d_rank = var_index[trigger[0]-data];
    var_index[trigger[1]-data] = d_rank;
    var_index[trigger[0]-data] = idx;
	
    *trigger[1] = *trigger[0];
    *trigger[0] = c;
	
    (*trigger[1])->index[d_rank] = trigger[1];

  } else {


    //std::cout << "here " << (trigger[0]-data) << std::endl;

    var_index[trigger[0]-data] = idx;
    *trigger[0] = c;

  }

  ++trigger[1];
      
  return trigger[0]++;
}
	  
Mistral::Constraint** Mistral::RangeTrigger::declare(Mistral::Constraint *c, const int idx) {

  Constraint **ptr;
  if(bound[1] < end) {
    ++bound[1];
    ptr = add(c, idx);
  } else if(bound[0] > data) {
    --bound[0];
    ptr = add_before(c, idx);
  } else {
    extend();
    --bound[0];
    ptr = add_before(c, idx);
  }
  return ptr;

}
*/



// Mistral::BranchingHeuristic *Mistral::Solver::heuristic_factory(std::string var_ordering, std::string branching) {

//   //std::cout << "% c  create heuristic " << var_ordering << " " << branching ; //<< std::endl;

//   BranchingHeuristic *heu = NULL;
//   if(var_ordering == "dom/wdeg") {
//     if(branching == "minval") {
//       heu = new GenericHeuristic < GenericWeightedDVO < FailureCountManager, MinDomainOverWeight >, MinValue > (this); 
//     } else if(branching == "maxval") {
//       heu = new GenericHeuristic < GenericWeightedDVO < FailureCountManager, MinDomainOverWeight >, MaxValue > (this); 
//     } else if(branching == "halfsplit") {
//       heu = new GenericHeuristic < GenericWeightedDVO < FailureCountManager, MinDomainOverWeight >, HalfSplit > (this); 
//     } else if(branching == "randminmax") {
//       heu = new GenericHeuristic < GenericWeightedDVO < FailureCountManager, MinDomainOverWeight >, RandomMinMax > (this); 
//     } else if(branching == "minweight") {
//       heu = new GenericHeuristic < GenericWeightedDVO < FailureCountManager, MinDomainOverWeight >, MinWeightValue > (this); 
//     } else if(branching == "guided") {
//       heu = new GenericHeuristic < GenericWeightedDVO < FailureCountManager, MinDomainOverWeight >, Guided > (this); 
//     } 
//   } else if(var_ordering == "dom/activity") {
//     if(branching == "minval") {
//       heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, MinValue > (this); 
//     } else if(branching == "maxval") {
//       heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, MaxValue > (this); 
//     } else if(branching == "halfsplit") {
//       heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, HalfSplit > (this); 
//     } else if(branching == "randminmax") {
//       heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, RandomMinMax > (this); 
//     } else if(branching == "minweight") {
//       heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, MinWeightValue > (this); 
//     } else if(branching == "guided") {
//       heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, Guided > (this); 
//     } 
//   } else if(var_ordering == "neighbor") {
//     if(branching == "minval") {
//       heu = new GenericHeuristic< GenericNeighborDVO< FailureCountManager, SelfPlusAverage, MinDomainOverWeight, 1>, MinValue > (this);
//       //heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, MinValue > (this); 
//     } else if(branching == "maxval") {
//       heu = new GenericHeuristic< GenericNeighborDVO< FailureCountManager, SelfPlusAverage, MinDomainOverWeight, 1>, MaxValue > (this);
//       //heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, MaxValue > (this); 
//     } else if(branching == "halfsplit") {
//       heu = new GenericHeuristic< GenericNeighborDVO< FailureCountManager, SelfPlusAverage, MinDomainOverWeight, 1>, HalfSplit > (this);
//       //heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, HalfSplit > (this); 
//     } else if(branching == "randminmax") {
//       heu = new GenericHeuristic< GenericNeighborDVO< FailureCountManager, SelfPlusAverage, MinDomainOverWeight, 1>, RandomMinMax > (this);
//       //heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, RandomMinMax > (this); 
//     } else if(branching == "minweight") {
//       heu = new GenericHeuristic< GenericNeighborDVO< FailureCountManager, SelfPlusAverage, MinDomainOverWeight, 1>, MinWeightValue > (this);
//       //heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, MinWeightValue > (this); 
//     } else if(branching == "guided") {
//       heu = new GenericHeuristic< GenericNeighborDVO< FailureCountManager, SelfPlusAverage, MinDomainOverWeight, 1>, Guided > (this);
//       //heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, Guided > (this); 
//     } 
//   } else if(var_ordering == "mindomain") {
//     if(branching == "minval") {
//       heu = new GenericHeuristic< GenericDVO< MinDomain >, MinValue > (this);
//       //heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, MinValue > (this); 
//     } else if(branching == "maxval") {
//       heu = new GenericHeuristic< GenericDVO< MinDomain >, MaxValue > (this);
//       //heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, MaxValue > (this); 
//     } else if(branching == "halfsplit") {
//       heu = new GenericHeuristic< GenericDVO< MinDomain >, HalfSplit > (this);
//       //heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, HalfSplit > (this); 
//     } else if(branching == "randminmax") {
//       heu = new GenericHeuristic< GenericDVO< MinDomain >, RandomMinMax > (this);
//       //heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, RandomMinMax > (this); 
//     } else if(branching == "guided") {
//       heu = new GenericHeuristic< GenericDVO< MinDomain >, Guided > (this);
//       //heu = new GenericHeuristic < GenericWeightedDVO < PruningCountManager, MinDomainOverWeight >, Guided > (this); 
//     } 
//   } else if(var_ordering == "lexicographic") {
//     if(branching == "minval") {
//       heu = new GenericHeuristic < Lexicographic, MinValue > (this); 
//     } else if(branching == "maxval") {
//       heu = new GenericHeuristic < Lexicographic, MaxValue > (this); 
//     } else if(branching == "halfsplit") {
//       heu = new GenericHeuristic < Lexicographic, HalfSplit > (this); 
//     } else if(branching == "randminmax") {
//       heu = new GenericHeuristic < Lexicographic, RandomMinMax > (this); 
//     } else if(branching == "guided") {
//       heu = new GenericHeuristic < Lexicographic, Guided > (this); 
//     } 
//   }	

//   if(!heu) {
//     std::cout << "% c Warning, there is no known heuristic \"" << var_ordering << "/" << branching << "\"" << std::endl;
//   }
					
//   return heu;
// }



Mistral::BranchingHeuristic *Mistral::Solver::heuristic_factory(std::string var_ordering, std::string branching, const int randomness) {

  BranchingHeuristic *heu = NULL;
  if(var_ordering == "dom/wdeg") {
    if(randomness < 2) {
      if(branching == "minval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, FailureCountManager >, MinValue > (this); 
      } else if(branching == "maxval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, FailureCountManager >, MaxValue > (this); 
      } else if(branching == "halfsplit") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, FailureCountManager >, HalfSplit > (this); 
      } else if(branching == "randminmax") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, FailureCountManager >, RandomMinMax > (this); 
      } else if(branching == "minweight") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, FailureCountManager >, MinWeightValue > (this); 
      } else if(branching == "guided") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, FailureCountManager >, Guided > (this); 
      } 
    } else if(randomness == 2) {
      if(branching == "minval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, FailureCountManager >, MinValue > (this); 
      } else if(branching == "maxval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, FailureCountManager >, MaxValue > (this); 
      } else if(branching == "halfsplit") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, FailureCountManager >, HalfSplit > (this); 
      } else if(branching == "randminmax") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, FailureCountManager >, RandomMinMax > (this); 
      } else if(branching == "minweight") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, FailureCountManager >, MinWeightValue > (this); 
      } else if(branching == "guided") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, FailureCountManager >, Guided > (this); 
      } 
    } else if(randomness == 3) {
      if(branching == "minval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, FailureCountManager >, MinValue > (this); 
      } else if(branching == "maxval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, FailureCountManager >, MaxValue > (this); 
      } else if(branching == "halfsplit") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, FailureCountManager >, HalfSplit > (this); 
      } else if(branching == "randminmax") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, FailureCountManager >, RandomMinMax > (this); 
      } else if(branching == "minweight") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, FailureCountManager >, MinWeightValue > (this); 
      } else if(branching == "guided") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, FailureCountManager >, Guided > (this); 
      } 
    } else {
      if(branching == "minval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, FailureCountManager >, MinValue > (this); 
      } else if(branching == "maxval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, FailureCountManager >, MaxValue > (this); 
      } else if(branching == "halfsplit") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, FailureCountManager >, HalfSplit > (this); 
      } else if(branching == "randminmax") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, FailureCountManager >, RandomMinMax > (this); 
      } else if(branching == "minweight") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, FailureCountManager >, MinWeightValue > (this); 
      } else if(branching == "guided") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, FailureCountManager >, Guided > (this); 
      } 
    }
  } else if(var_ordering == "dom/activity") {
    if(randomness < 2) {
      if(branching == "minval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, PruningCountManager >, MinValue > (this); 
      } else if(branching == "maxval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, PruningCountManager >, MaxValue > (this); 
      } else if(branching == "halfsplit") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, PruningCountManager >, HalfSplit > (this); 
      } else if(branching == "randminmax") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, PruningCountManager >, RandomMinMax > (this); 
      } else if(branching == "minweight") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, PruningCountManager >, MinWeightValue > (this); 
      } else if(branching == "guided") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 1, PruningCountManager >, Guided > (this); 
      } 
    } else if(randomness == 2) {
      if(branching == "minval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, PruningCountManager >, MinValue > (this); 
      } else if(branching == "maxval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, PruningCountManager >, MaxValue > (this); 
      } else if(branching == "halfsplit") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, PruningCountManager >, HalfSplit > (this); 
      } else if(branching == "randminmax") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, PruningCountManager >, RandomMinMax > (this); 
      } else if(branching == "minweight") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, PruningCountManager >, MinWeightValue > (this); 
      } else if(branching == "guided") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 2, PruningCountManager >, Guided > (this); 
      } 
    } else if(randomness == 3) {
      if(branching == "minval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, PruningCountManager >, MinValue > (this); 
      } else if(branching == "maxval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, PruningCountManager >, MaxValue > (this); 
      } else if(branching == "halfsplit") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, PruningCountManager >, HalfSplit > (this); 
      } else if(branching == "randminmax") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, PruningCountManager >, RandomMinMax > (this); 
      } else if(branching == "minweight") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, PruningCountManager >, MinWeightValue > (this); 
      } else if(branching == "guided") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 3, PruningCountManager >, Guided > (this); 
      } 
    } else {
      if(branching == "minval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, PruningCountManager >, MinValue > (this); 
      } else if(branching == "maxval") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, PruningCountManager >, MaxValue > (this); 
      } else if(branching == "halfsplit") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, PruningCountManager >, HalfSplit > (this); 
      } else if(branching == "randminmax") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, PruningCountManager >, RandomMinMax > (this); 
      } else if(branching == "minweight") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, PruningCountManager >, MinWeightValue > (this); 
      } else if(branching == "guided") {
	heu = new GenericHeuristic < GenericDVO < MinDomainOverWeight, 5, PruningCountManager >, Guided > (this); 
      } 
    }
  } else if(var_ordering == "neighbor") {
    if(branching == "minval") {
      heu = new GenericHeuristic< GenericNeighborDVO< SelfPlusAverage, MinDomainOverWeight, 1, FailureCountManager >, MinValue > (this);
    } else if(branching == "maxval") {
      heu = new GenericHeuristic< GenericNeighborDVO< SelfPlusAverage, MinDomainOverWeight, 1, FailureCountManager >, MaxValue > (this);
    } else if(branching == "halfsplit") {
      heu = new GenericHeuristic< GenericNeighborDVO< SelfPlusAverage, MinDomainOverWeight, 1, FailureCountManager >, HalfSplit > (this);
    } else if(branching == "randminmax") {
      heu = new GenericHeuristic< GenericNeighborDVO< SelfPlusAverage, MinDomainOverWeight, 1, FailureCountManager >, RandomMinMax > (this);
    } else if(branching == "minweight") {
      heu = new GenericHeuristic< GenericNeighborDVO< SelfPlusAverage, MinDomainOverWeight, 1, FailureCountManager >, MinWeightValue > (this);
    } else if(branching == "guided") {
      heu = new GenericHeuristic< GenericNeighborDVO< SelfPlusAverage, MinDomainOverWeight, 1, FailureCountManager >, Guided > (this);
    } 
  } else if(var_ordering == "mindomain") {
    if(branching == "minval") {
      heu = new GenericHeuristic< GenericDVO< MinDomain >, MinValue > (this);
    } else if(branching == "maxval") {
      heu = new GenericHeuristic< GenericDVO< MinDomain >, MaxValue > (this);
    } else if(branching == "halfsplit") {
      heu = new GenericHeuristic< GenericDVO< MinDomain >, HalfSplit > (this);
    } else if(branching == "randminmax") {
      heu = new GenericHeuristic< GenericDVO< MinDomain >, RandomMinMax > (this);
    } else if(branching == "guided") {
      heu = new GenericHeuristic< GenericDVO< MinDomain >, Guided > (this);
    } 
  } else if(var_ordering == "maxdegree") {
    if(branching == "minval") {
      heu = new GenericHeuristic< GenericDVO< MaxDegree >, MinValue > (this);
    } else if(branching == "maxval") {
      heu = new GenericHeuristic< GenericDVO< MaxDegree >, MaxValue > (this);
    } else if(branching == "halfsplit") {
      heu = new GenericHeuristic< GenericDVO< MaxDegree >, HalfSplit > (this);
    } else if(branching == "randminmax") {
      heu = new GenericHeuristic< GenericDVO< MaxDegree >, RandomMinMax > (this);
    } else if(branching == "guided") {
      heu = new GenericHeuristic< GenericDVO< MaxDegree >, Guided > (this);
    } 
  } else if(var_ordering == "dom/deg") {
    if(branching == "minval") {
      heu = new GenericHeuristic< GenericDVO< MinDomainOverDegree >, MinValue > (this);
    } else if(branching == "maxval") {
      heu = new GenericHeuristic< GenericDVO< MinDomainOverDegree >, MaxValue > (this);
    } else if(branching == "halfsplit") {
      heu = new GenericHeuristic< GenericDVO< MinDomainOverDegree >, HalfSplit > (this);
    } else if(branching == "randminmax") {
      heu = new GenericHeuristic< GenericDVO< MinDomainOverDegree >, RandomMinMax > (this);
    } else if(branching == "guided") {
      heu = new GenericHeuristic< GenericDVO< MinDomainOverDegree >, Guided > (this);
    } 
  } else if(var_ordering == "lexicographic") {
    if(branching == "minval") {
      heu = new GenericHeuristic < Lexicographic, MinValue > (this); 
    } else if(branching == "maxval") {
      heu = new GenericHeuristic < Lexicographic, MaxValue > (this); 
    } else if(branching == "halfsplit") {
      heu = new GenericHeuristic < Lexicographic, HalfSplit > (this); 
    } else if(branching == "randminmax") {
      heu = new GenericHeuristic < Lexicographic, RandomMinMax > (this); 
    } else if(branching == "guided") {
      heu = new GenericHeuristic < Lexicographic, Guided > (this); 
    } 
  } else if(var_ordering == "input_order") {
    if(branching == "indomain_min") {
      heu = new GenericHeuristic < Lexicographic, MinValue > (this); 
    } else if(branching == "indomain_max") {
      heu = new GenericHeuristic < Lexicographic, MaxValue > (this); 
    } else if(branching == "indomain_middle") {
      heu = new GenericHeuristic < Lexicographic, MiddleValue > (this); 
    } else if(branching == "indomain_median") {
      heu = new GenericHeuristic < Lexicographic, MedianValue > (this); 
    }//  else if(branching == "indomain") {
    //   heu = new GenericHeuristic < Lexicographic, Any? > (this); 
    // } 
    else if(branching == "indomain_random") {
      heu = new GenericHeuristic < Lexicographic, RandomValue > (this); 
    } else if(branching == "indomain_split") {
      heu = new GenericHeuristic < Lexicographic, HalfSplit > (this);
    } else if(branching == "indomain_reverse_split") {
      heu = new GenericHeuristic < Lexicographic, ReverseSplit > (this);
    } else if(branching == "indomain_interval") {
      heu = new GenericHeuristic < Lexicographic, HalfSplit > (this);
    }
  } else if(var_ordering == "first_fail") {
    if(branching == "indomain_min") {
      heu = new GenericHeuristic < GenericDVO< MinDomain >, MinValue > (this); 
    } else if(branching == "indomain_max") {
      heu = new GenericHeuristic < GenericDVO< MinDomain >, MaxValue > (this); 
    } else if(branching == "indomain_middle") {
      heu = new GenericHeuristic < GenericDVO< MinDomain >, MiddleValue > (this); 
    } else if(branching == "indomain_median") {
      heu = new GenericHeuristic < GenericDVO< MinDomain >, MedianValue > (this); 
    }//  else if(branching == "indomain") {
    //   heu = new GenericHeuristic < GenericDVO< MinDomain >, Any? > (this); 
    // } 
    else if(branching == "indomain_random") {
      heu = new GenericHeuristic < GenericDVO< MinDomain >, RandomValue > (this); 
    } else if(branching == "indomain_split") {
      heu = new GenericHeuristic < GenericDVO< MinDomain >, HalfSplit > (this);
    } else if(branching == "indomain_reverse_split") {
      heu = new GenericHeuristic < GenericDVO< MinDomain >, ReverseSplit > (this);
    } else if(branching == "indomain_interval") {
      heu = new GenericHeuristic < GenericDVO< MinDomain >, HalfSplit > (this);
    }
  } else if(var_ordering == "anti_first_fail") {
    if(branching == "indomain_min") {
      heu = new GenericHeuristic < GenericDVO< Anti<MinDomain> >, MinValue > (this); 
    } else if(branching == "indomain_max") {
      heu = new GenericHeuristic < GenericDVO< Anti<MinDomain> >, MaxValue > (this); 
    } else if(branching == "indomain_middle") {
      heu = new GenericHeuristic < GenericDVO< Anti<MinDomain> >, MiddleValue > (this); 
    } else if(branching == "indomain_median") {
      heu = new GenericHeuristic < GenericDVO< Anti<MinDomain> >, MedianValue > (this); 
    }//  else if(branching == "indomain") {
    //   heu = new GenericHeuristic < GenericDVO< Anti<MinDomain> >, Any? > (this); 
    // } 
    else if(branching == "indomain_random") {
      heu = new GenericHeuristic < GenericDVO< Anti<MinDomain> >, RandomValue > (this); 
    } else if(branching == "indomain_split") {
      heu = new GenericHeuristic < GenericDVO< Anti<MinDomain> >, HalfSplit > (this);
    } else if(branching == "indomain_reverse_split") {
      heu = new GenericHeuristic < GenericDVO< Anti<MinDomain> >, ReverseSplit > (this);
    } else if(branching == "indomain_interval") {
      heu = new GenericHeuristic < GenericDVO< Anti<MinDomain> >, HalfSplit > (this);
    }
  } else if(var_ordering == "smallest") {
    if(branching == "indomain_min") {
      heu = new GenericHeuristic < GenericDVO< MinMin >, MinValue > (this); 
    } else if(branching == "indomain_max") {
      heu = new GenericHeuristic < GenericDVO< MinMin >, MaxValue > (this); 
    } else if(branching == "indomain_middle") {
      heu = new GenericHeuristic < GenericDVO< MinMin >, MiddleValue > (this); 
    } else if(branching == "indomain_median") {
      heu = new GenericHeuristic < GenericDVO< MinMin >, MedianValue > (this); 
    }//  else if(branching == "indomain") {
    //   heu = new GenericHeuristic < GenericDVO< MinMin >, Any? > (this); 
    // } 
    else if(branching == "indomain_random") {
      heu = new GenericHeuristic < GenericDVO< MinMin >, RandomValue > (this); 
    } else if(branching == "indomain_split") {
      heu = new GenericHeuristic < GenericDVO< MinMin >, HalfSplit > (this);
    } else if(branching == "indomain_reverse_split") {
      heu = new GenericHeuristic < GenericDVO< MinMin >, ReverseSplit > (this);
    } else if(branching == "indomain_interval") {
      heu = new GenericHeuristic < GenericDVO< MinMin >, HalfSplit > (this);
    }
  }  else if(var_ordering == "largest") {
     if(branching == "indomain_min") {
       heu = new GenericHeuristic < GenericDVO< MaxMax >, MinValue > (this);
     } else if(branching == "indomain_max") {
       heu = new GenericHeuristic < GenericDVO< MaxMax >, MaxValue > (this);
     } else if(branching == "indomain_middle") {
       heu = new GenericHeuristic < GenericDVO< MaxMax >, MiddleValue > (this);
     } else if(branching == "indomain_median") {
       heu = new GenericHeuristic < GenericDVO< MaxMax >, MedianValue > (this);
     }//  else if(branching == "indomain") {
     //   heu = new GenericHeuristic < GenericDVO< MaxMax >, Any? > (this);
     // }
     else if(branching == "indomain_random") {
       heu = new GenericHeuristic < GenericDVO< MaxMax >, RandomValue > (this);
     } else if(branching == "indomain_split") {
       heu = new GenericHeuristic < GenericDVO< MaxMax >, HalfSplit > (this);
     } else if(branching == "indomain_reverse_split") {
       heu = new GenericHeuristic < GenericDVO< MaxMax >, ReverseSplit > (this);
     } else if(branching == "indomain_interval") {
       heu = new GenericHeuristic < GenericDVO< MaxMax >, HalfSplit > (this);
     }
   }
  else if(var_ordering == "occurrence") {
    if(branching == "indomain_min") {
      heu = new GenericHeuristic < GenericDVO< MaxDegree >, MinValue > (this); 
    } else if(branching == "indomain_max") {
      heu = new GenericHeuristic < GenericDVO< MaxDegree >, MaxValue > (this); 
    } else if(branching == "indomain_middle") {
      heu = new GenericHeuristic < GenericDVO< MaxDegree >, MiddleValue > (this); 
    } else if(branching == "indomain_median") {
      heu = new GenericHeuristic < GenericDVO< MaxDegree >, MedianValue > (this); 
    }//  else if(branching == "indomain") {
    //   heu = new GenericHeuristic < GenericDVO< MaxDegree >, Any? > (this); 
    // } 
    else if(branching == "indomain_random") {
      heu = new GenericHeuristic < GenericDVO< MaxDegree >, RandomValue > (this); 
    } else if(branching == "indomain_split") {
      heu = new GenericHeuristic < GenericDVO< MaxDegree >, HalfSplit > (this);
    } else if(branching == "indomain_reverse_split") {
      heu = new GenericHeuristic < GenericDVO< MaxDegree >, ReverseSplit > (this);
    } else if(branching == "indomain_interval") {
      heu = new GenericHeuristic < GenericDVO< MaxDegree >, HalfSplit > (this);
    }
  } else if(var_ordering == "most_constrained") {
    if(branching == "indomain_min") {
      heu = new GenericHeuristic < GenericDVO< MinDomainMaxDegree >, MinValue > (this); 
    } else if(branching == "indomain_max") {
      heu = new GenericHeuristic < GenericDVO< MinDomainMaxDegree >, MaxValue > (this); 
    } else if(branching == "indomain_middle") {
      heu = new GenericHeuristic < GenericDVO< MinDomainMaxDegree >, MiddleValue > (this); 
    } else if(branching == "indomain_median") {
      heu = new GenericHeuristic < GenericDVO< MinDomainMaxDegree >, MedianValue > (this); 
    }//  else if(branching == "indomain") {
    //   heu = new GenericHeuristic < GenericDVO< MinDomainMaxDegree >, Any? > (this); 
    // } 
    else if(branching == "indomain_random") {
      heu = new GenericHeuristic < GenericDVO< MinDomainMaxDegree >, RandomValue > (this); 
    } else if(branching == "indomain_split") {
      heu = new GenericHeuristic < GenericDVO< MinDomainMaxDegree >, HalfSplit > (this);
    } else if(branching == "indomain_reverse_split") {
      heu = new GenericHeuristic < GenericDVO< MinDomainMaxDegree >, ReverseSplit > (this);
    } else if(branching == "indomain_interval") {
      heu = new GenericHeuristic < GenericDVO< MinDomainMaxDegree >, HalfSplit > (this);
    }
  } else if(var_ordering == "max_regret") {
    if(branching == "indomain_min") {
      heu = new GenericHeuristic < GenericDVO< MaxRegret >, MinValue > (this); 
    } else if(branching == "indomain_max") {
      heu = new GenericHeuristic < GenericDVO< MaxRegret >, MaxValue > (this); 
    } else if(branching == "indomain_middle") {
      heu = new GenericHeuristic < GenericDVO< MaxRegret >, MiddleValue > (this); 
    } else if(branching == "indomain_median") {
      heu = new GenericHeuristic < GenericDVO< MaxRegret >, MedianValue > (this); 
    }//  else if(branching == "indomain") {
    //   heu = new GenericHeuristic < GenericDVO< MaxRegret >, Any? > (this); 
    // } 
    else if(branching == "indomain_random") {
      heu = new GenericHeuristic < GenericDVO< MaxRegret >, RandomValue > (this); 
    } else if(branching == "indomain_split") {
      heu = new GenericHeuristic < GenericDVO< MaxRegret >, HalfSplit > (this);
    } else if(branching == "indomain_reverse_split") {
      heu = new GenericHeuristic < GenericDVO< MaxRegret >, ReverseSplit > (this);
    } else if(branching == "indomain_interval") {
      heu = new GenericHeuristic < GenericDVO< MaxRegret >, HalfSplit > (this);
    }
  }

  if(!heu) {
    std::cout << "% c Warning, there is no known heuristic \"" << var_ordering << "/" << branching << "\"" << std::endl;
  }
					
  return heu;
}

Mistral::RestartPolicy *Mistral::Solver::restart_factory(std::string rpolicy) {
  RestartPolicy *pol;
  if(rpolicy == "luby") pol = new Luby(); 
  else if(rpolicy == "geom") pol = new Geometric(); 
  else pol = new NoRestart();
  return pol;
}


void Mistral::Solver::initialise_random_seed(const int seed) {
  usrand(seed);
}


void Mistral::Solver::set_time_limit(const double limit) {
  if(limit > 0) {
    parameters.limit = 1;
    parameters.time_limit = limit;
  }
}


void Mistral::Solver::check_constraint_graph_integrity() {
  // for each constraint, check if the set of active variables corresponds to unbound vars
  for(unsigned int i=0; i<constraints.size; ++i) {
    //if(constraints[i].is_active())
    constraints[i].check_active();
  }

  // for each variable, check that
  // 1/ no constraint is listed twice
  // 2/ for each constraint in the list for this var:
  //   a/ the index of the constraint for that var corresponds to the rank of var in its scope
  //   b/ the constraint has at least 2 unbound vars, or does not enforce nfc1
  BitSet cons_list(0, constraints.size, BitSet::empt);
  int trig, cons;
  Constraint c;
  Variable *scope;
  for(unsigned int i=0; i<variables.size; ++i) {
    cons_list.clear();

    for(trig = _VALUE_; trig<=_DOMAIN_; ++trig) {

	for(cons = constraint_graph[i].on[trig].size; --cons>=0;) {

	  c = constraint_graph[i].on[trig][cons];

	  if(cons_list.contain(c.id())) {

	    std::cout << "Warning: " << c << " is listed at least twice: " 
		      << std::endl << constraint_graph[i].on[0] 
		      << std::endl << constraint_graph[i].on[1] 
		      << std::endl << constraint_graph[i].on[2] << std::endl;

	    exit(1);
	    
	  } else {
	    cons_list.add(c.id());
	  }

	  scope = c.get_scope();
	  
	  if(scope[c.index()].id() != (int)i) {

	    std::cout << "Warning: incorrect variable indexing: " 
		      << scope[c.index()] << "'s " << c
		      << " is posted on " << variables[i] << std::endl;

	    exit(1);

	  }

	  if(c.rank() != cons) {

	    std::cout << "Warning: incorrect list indexing: " << c << " is " 
		      << cons << "th in the list of " << variables[i] 
		      << ", but indexed " << c.rank() << "th." << std::endl;  

	    exit(1);
	  }
	}
    }
  }
}
