
#include <stdlib.h>
#include <fstream>
#include <vector>
#include <map>


#include <mistral_solver.hpp>
#include <mistral_variable.hpp>
#include <mistral_search.hpp>

#include <tclap/CmdLine.h>


using namespace std;
using namespace Mistral;
using namespace TCLAP;

#define CS_RAND_MAX 1000

// #define _PRINT_SEARCH true
// #define _PRINT_CLASSES true
// #define _DEBUG_HEURISTIC true


class CarSequencingInstance {

private:
  
  int length;
  vector < vector< int > > configuration;
  vector < vector< int > > option_config;
  vector < BitSet >        config_matrix;
  vector < int >           option_demand;
  vector < int >            class_demand;
  vector < int >              capacity_p;
  vector < int >              capacity_q;

public:
  
  CarSequencingInstance(const char *filename);
  virtual ~CarSequencingInstance();
  
  // total number of cars to produce
  int nb_cars() const;
  // total number of classes of cars
  int nb_classes() const;
  // total number of options for cars
  int nb_options() const;

  // number of cars of class c
  int nb_cars_of_class(const int c) const;
  // number of cars with option o
  int nb_cars_with_option(const int o) const;
  // number of options of that class
  int nb_options_of_class(const int c) const;
  
  // sequence length for option o
  int sequence_length(const int o) const;
  // capacity max for option o
  int max_capacity(const int o) const;

  // does class c requires option o
  bool has_option(const int c, const int o) const;

  const vector<int>& get_classes_of_option(const int o) const;


  // get the options of class c
  vector< int >::iterator obegin(const int c) ;
  vector< int >::iterator oend(const int c) ;

  // get the classes requiring option o
  vector< int >::iterator cbegin(const int o) ;
  vector< int >::iterator cend(const int o) ;

  // get the demand of options
  vector< int >::iterator option_demand_begin();
  vector< int >::iterator option_demand_end();
  
  // get the demand of classes
  vector< int >::iterator class_demand_begin();
  vector< int >::iterator class_demand_end();

  // get the max capacity
  vector< int >::iterator pbegin() ;
  vector< int >::iterator pend();

 // get the subsequence length
  vector< int >::iterator qbegin();
  vector< int >::iterator qend();


  // printing
  ostream& display(ostream& os) const;

};


CarSequencingInstance::CarSequencingInstance(const char *filename) {
  ifstream fin(filename,ios::in);
  if (!fin) cout << "problem with file:" << filename << endl;

  int opt, cla, aux;
  int nbOptions;
  int nbClasses;
  fin >> length >> nbOptions >> nbClasses;

  capacity_p.resize(nbOptions);
  capacity_q.resize(nbOptions);

  option_config.resize(nbOptions);
  option_demand.resize(nbOptions);
 
  config_matrix.resize(nbClasses);
  configuration.resize(nbClasses);

  class_demand.resize(nbClasses);

  for(cla=0;cla<nbClasses;++cla) 
    config_matrix[cla].initialise(0, nbOptions-1, BitSet::empt);

  // read the maximum number of cars of each sequence that can take the invoked option
  for(opt=0;opt<nbOptions;++opt){
    fin >> aux;
    capacity_p[opt] = aux;
  }

  // read the size of a sequence for each option
  for(opt=0;opt<nbOptions;++opt){
    fin >> aux;
    capacity_q[opt] = aux;
  }

  // read the options required by each configuration
  for(cla=0;cla<nbClasses;++cla){
    fin >> aux;
    fin >> aux;
    class_demand[cla] = aux;

    //nbCarsByClass[cla];
    for (opt=0;opt<nbOptions;++opt){
      fin >> aux;
      if(aux) {
	configuration[cla].push_back(opt);
	option_config[opt].push_back(cla);
	config_matrix[cla].add(opt);
      }
    }
  }

  // compute the number of cars to be built for each option
  for(opt=0;opt<nbOptions;++opt){
    aux = 0;
    vector< int >::iterator last_i = cend(opt);
    vector< int >::iterator class_i = cbegin(opt);
    for(; class_i!=last_i;++class_i) {
      aux += nb_cars_of_class(*class_i);   
    } 
    option_demand[opt] = aux;
  }
}

CarSequencingInstance::~CarSequencingInstance() {}

ostream& operator<<(ostream& os, const CarSequencingInstance inst) {
 return  inst.display(os);
}

ostream& CarSequencingInstance::display(ostream& os) const {
  int opt, cla;
  os << left << " c  Cars  (" << setw(4) << nb_cars() << ")" << endl
     << left << " c  Options (" << setw(2) << nb_options() << left << setw(9) << "): " ;
  for(opt=0; opt<nb_options(); ++opt) 
    os << right << setw(4) << max_capacity(opt);
  os << endl << setw(24) << " ";
  for(opt=0; opt<nb_options(); ++opt) 
    os << right << setw(4) << sequence_length(opt);
  os << endl << setw(24) << " ";
  for(opt=0; opt<nb_options(); ++opt) 
    os << right << setw(4) << nb_cars_with_option(opt);
  os << endl;
  os << " c  Classes (" << right << setw(2) << nb_classes() << "):\n" ;
  for(cla=0;cla<nb_classes();++cla) {
    os << " c  " << setw(12) << cla << ":"  << setw(4) << nb_cars_of_class(cla) << " of " << config_matrix[cla] << endl;
  }
  os << endl;
  return os;
}


const vector<int>& CarSequencingInstance::get_classes_of_option(const int o) const {
  return option_config[o];
}

// total number of cars
int CarSequencingInstance::nb_cars() const {
  return length;
}

// total number of classes
int CarSequencingInstance::nb_classes() const {
  return configuration.size();
}

// total number of options
int CarSequencingInstance::nb_options() const {
  return option_config.size();
}


// number of cars of class c 
int CarSequencingInstance::nb_cars_of_class(const int c) const {
  return class_demand[c];
}

// number of cars with option o
int CarSequencingInstance::nb_cars_with_option(const int o) const {
  return option_demand[o];
}

// number of options of that class
int CarSequencingInstance::nb_options_of_class(const int c) const {
  return configuration[c].size();
}
  
// sequence length for option o
int CarSequencingInstance::sequence_length(const int o) const {
  return capacity_q[o];
}

// capacity max for option o
int CarSequencingInstance::max_capacity(const int o) const {
  return capacity_p[o];
}

// does class c requires option o
bool CarSequencingInstance::has_option(const int c, const int o) const {
  return config_matrix[c].contain(o);
}

// get the options of class c
vector< int >::iterator CarSequencingInstance::obegin(const int c)  {
  return configuration[c].begin();
}
vector< int >::iterator CarSequencingInstance::oend(const int c)  {
  return configuration[c].end();
}

// get the options of class o
vector< int >::iterator CarSequencingInstance::cbegin(const int o)  {
  return option_config[o].begin();
}
vector< int >::iterator CarSequencingInstance::cend(const int o)  {
  return option_config[o].end();
}

// get the demand of options
vector< int >::iterator CarSequencingInstance::option_demand_begin() {
  return option_demand.begin();
}
vector< int >::iterator CarSequencingInstance::option_demand_end() {
  return option_demand.end();
}

// get the demand of classes
vector< int >::iterator CarSequencingInstance::class_demand_begin() {
  return class_demand.begin();
}
vector< int >::iterator CarSequencingInstance::class_demand_end() {
  return class_demand.end();
}

// get the max capacity
vector< int >::iterator CarSequencingInstance::pbegin()  {
  return capacity_p.begin();
}
vector< int >::iterator CarSequencingInstance::pend() {
  return capacity_p.end();
}

// get the subsequence length
vector< int >::iterator CarSequencingInstance::qbegin() {
  return capacity_q.begin();
}
vector< int >::iterator CarSequencingInstance::qend() {
  return capacity_q.end();
}


class CarSequencingModel : public Solver {

public:
  
  // pointer to the instance
  CarSequencingInstance *instance;

  // variables for classes
  VarArray class_at_position;
  
  // variables for options
  vector< VarArray > option;

  // Boolean representation of classes
  vector< VarArray > bool_class;



  CarSequencingModel(CarSequencingInstance *inst) : Solver(), instance(inst) {}
  virtual ~CarSequencingModel() {}

  virtual void setup();

  int get_opt_index(const int idx) {
    int opt = idx-instance->nb_cars(); // remove the class variables
    opt /= idx-instance->nb_cars();
    return opt;
  }

  int get_car_index(const int idx) {
    int car = idx-instance->nb_cars(); // remove the class variables
    car %= instance->nb_cars();
    return car;
  }

};

void CarSequencingModel::setup() {

  // class variables
  class_at_position.initialise(instance->nb_cars(), 0, instance->nb_classes()-1);
  add(class_at_position);

  // option variables
  option.resize(instance->nb_options());
  for(int opt=0; opt<instance->nb_options(); ++opt) {
    // channeling class <-> option
    for(int i=0; i<instance->nb_cars(); ++i) {
      option[opt].add( Member(class_at_position[i], instance->get_classes_of_option(opt)) );
      add( Free(option[opt].back()) );
    }

    //add( Free(option[opt]) );
  }

  // bool class variables
  bool_class.resize(instance->nb_classes());
  for(int cla=0; cla<instance->nb_classes(); ++cla) {
    // channeling class <-> bool_class
    for(int i=0; i<instance->nb_cars(); ++i) {
      bool_class[cla].add(class_at_position[i] == cla);
      Free(bool_class[cla].back());
    }

    //add( Free(bool_class[cla]) );
  }
}


class SumModel : public CarSequencingModel {

public:

  SumModel(CarSequencingInstance *inst) : CarSequencingModel(inst) {}
  virtual ~SumModel() {}

  virtual void setup();

};

void SumModel::setup() {
  CarSequencingModel::setup();

  for(int opt=0; opt<instance->nb_options(); ++opt) {
    // demand on options as boolean sums too
    add( BoolSum(option[opt], instance->nb_cars_with_option(opt)) );

    // capacity constraint as boolean sums
    int p = instance->max_capacity(opt);
    int q = instance->sequence_length(opt);
    int n = instance->nb_cars()-q+1;
    VarArray subsequence;
    for(int i=0; i<n; ++i) {
      subsequence.clear();
      for(int j=0; j<q; ++j) subsequence.add(option[opt][i+j]);    
      add( BoolSum( subsequence, -INFTY, p ) );
    }
  }

  for(int cla=0; cla<instance->nb_classes(); ++cla) {
    // demand
    add( BoolSum(bool_class[cla], instance->nb_cars_of_class(cla)) );
  }
}


class SumAmscModel : public SumModel {

public:

  SumAmscModel(CarSequencingInstance *inst) : SumModel(inst) {}
  virtual ~SumAmscModel() {}

  virtual void setup();

};

void SumAmscModel::setup() {
  SumModel::setup();
  for(int opt=0; opt<instance->nb_options(); ++opt) {
    // demand on options as boolean sums too
    add( AtMostSeqCard( option[opt], 
			instance->nb_cars_with_option(opt), 
			instance->max_capacity(opt), 
			instance->sequence_length(opt) ) );
  }
}



class SumMultiAmscModel : public SumAmscModel {

public:

  SumMultiAmscModel(CarSequencingInstance *inst) : SumAmscModel(inst) {}
  virtual ~SumMultiAmscModel() {}

  virtual void setup();

};

void SumMultiAmscModel::setup() {
  SumAmscModel::setup();
  for(int cla=0; cla<instance->nb_classes(); ++cla) {

    Vector< Tuple< 2, int > > capacity;
    vector< int >::iterator opt_it = instance->obegin(cla);
    vector< int >::iterator opt_end = instance->oend(cla);

    for(; opt_it != opt_end; ++opt_it) {
      Tuple< 2, int > capa(instance->max_capacity(*opt_it), 
			   instance->sequence_length(*opt_it));
      capacity.add(capa);
    }

    add( MultiAtMostSeqCard( bool_class[cla], 
			     instance->nb_cars_of_class(cla), 
			     capacity ) );
  }


  for(int cla=0; cla<instance->nb_classes(); ++cla) {
    if(instance->nb_options_of_class(cla) > 1) {
      // compute a maximal set sharing at least
      
    }
  }



}



// class OptionDemand {

// public:
//   int nbOptions;
//   vector<int>::iterator initial_demand;
//   double *residual_demand;
  
//   OptionDemand() { }; //instance = NULL; }
//   virtual ~OptionDemand() { }; 

//   void initialise(CarSequencingInstance *instance, double *opt_weight) {
//     nbOptions = instance->nb_options();
//     residual_demand = opt_weight;
//     initial_demand = instance->option_demand_begin();
//   }

//   void update(CarSequencingModel *solver) {
//     int supply, opt;
//     Vector< Variable >::iterator x_it;
//     Vector< Variable >::iterator stop;
//     vector< int >::iterator demand_it;

//     for(demand_it = initial_demand, opt=0; opt<nbOptions; ++opt, ++demand_it) {

//       //cout << "option_" << opt << " demand=" << *demand_it ; 

//       for(supply=0, stop = solver->option[opt].end(), x_it = solver->option[opt].begin(); x_it!=stop; ++x_it) {
// 	supply += x_it->get_min();
//       }
//       residual_demand[opt] = *demand_it - supply;

//       //cout << " supply=" << supply << " => " << residual_demand[opt] << endl;
//     }
//   }
// };


class StaticCapacity {

public:
  
  StaticCapacity() { }; //instance = NULL; }
  virtual ~StaticCapacity() { }; 

  void initialise(CarSequencingInstance *instance, double *opt_weight) {
    vector< int >::iterator pit = instance->pbegin();
    vector< int >::iterator qit = instance->qbegin();

    for(int opt=0; opt<instance->nb_options(); ++opt, ++pit, ++qit) {
      opt_weight[opt] = (double)(*pit) / (double)(*qit);
    }
  }

  void update(CarSequencingModel *solver) {}
};



struct Demand {
  void operator() (double& result, const int d, const int p, const int q, const int n, const int N) {
    result = (double)(d);
  }
};

struct Load {
  void operator() (double& result, const int d, const int p, const int q, const int n, const int N) {
    result = (double)(d*q)/(double)p;
  }
};

struct Rate {
  void operator() (double& result, const int d, const int p, const int q, const int n, const int N) {
    result = (double)(d*q)/(double)(p*n);
  }
};

struct Slack {
  void operator() (double& result, const int d, const int p, const int q, const int n, const int N) {
    result =  (double)N + ((double)(d*q)/(double)p)-(double)n;
  }
};



template<typename Function>
class Dynamic {

public:
  int nbOptions;
  int nbCars;
  vector<int>::iterator demand;
  vector<int>::iterator p;
  vector<int>::iterator q;
  double *score;
  
  Dynamic() { }; //instance = NULL; }
  virtual ~Dynamic() { }; 

  void initialise(CarSequencingInstance *instance, double *opt_weight) {
    nbOptions = instance->nb_options();
    nbCars = instance->nb_cars();
    score = opt_weight;
    demand = instance->option_demand_begin();
    p = instance->pbegin();
    q = instance->qbegin();
  }

  void update(CarSequencingModel *solver) {
    int dem, opt, slot;
    Vector< Variable >::iterator x_it;
    Vector< Variable >::iterator stop;

    vector< int >::iterator demand_it;
    vector< int >::iterator p_it;
    vector< int >::iterator q_it;


    for(demand_it = demand, p_it = p, q_it = q, opt=0; opt<nbOptions; ++opt, ++demand_it, ++p_it, ++q_it) {

      // cout << "option_" << opt << ": ";
      // cout.flush();

      for(slot = nbCars, dem = *demand_it, stop = solver->option[opt].end(), x_it = solver->option[opt].begin(); x_it!=stop; ++x_it) {
	
#ifdef _PRINT_SEARCH
	if(x_it->is_ground()) cout << x_it->get_min();
	else cout << ".";
#endif

	dem -= x_it->get_min();
	slot -= x_it->is_ground();
      }
#ifdef _PRINT_SEARCH
      cout << endl;
#endif

      //score[opt] = 
      if(slot)
	Function()(score[opt], dem, *p_it, *q_it, slot, nbCars);
      else
	score[opt] = 0;
	//(double)(*q_it) * (double)dem / (double)(*p_it);

      //cout << dem << " " << *p_it << " " << *q_it << " " << slot << " " << nbCars << " ==> " << score[opt] << endl;

    }
#ifdef _PRINT_SEARCH
    cout << endl;
#endif
  }
};





// class OptionLoad {

// public:
//   int nbOptions;
//   int nbCars;
//   vector<int>::iterator demand;
//   vector<int>::iterator p;
//   vector<int>::iterator q;
//   double *score;
  
//   OptionLoad() { }; //instance = NULL; }
//   virtual ~OptionLoad() { }; 

//   void initialise(CarSequencingInstance *instance, double *opt_weight) {
//     nbOptions = instance->nb_options();
//     nbCars = instance->nb_cars();
//     score = opt_weight;
//     demand = instance->option_demand_begin();
//     p = instance->pbegin();
//     q = instance->qbegin();
//   }

//   void update(CarSequencingModel *solver) {
//     int dem, opt;//, slot;
//     Vector< Variable >::iterator x_it;
//     Vector< Variable >::iterator stop;

//     vector< int >::iterator demand_it;
//     vector< int >::iterator p_it;
//     vector< int >::iterator q_it;

//     for(demand_it = demand, p_it = p, q_it = q, opt=0; opt<nbOptions; ++opt, ++demand_it, ++p_it, ++q_it) {
//       for(// slot = nbCars, 
// 	    dem = *demand_it, stop = solver->option[opt].end(), x_it = solver->option[opt].begin(); x_it!=stop; ++x_it) {
// 	dem -= x_it->get_min();
// 	//slot -= x_it->is_ground();
//       }
//       score[opt] = 
//       //Function()(score[opt], dem, *p_it, *q_it, slot, nbCars);
// 	(double)(*q_it) * (double)dem / (double)(*p_it);
//     }
//   }
// };



class MaxSumOf {

public:

  MaxSumOf() { clear(); }
  virtual ~MaxSumOf() {}

  double value;
  
  void clear() { value = -INFTY; }

  void operator=( vector<double>& x) {
    value = 0;
    vector<double>::iterator stop = x.end();
    for( vector<double>::iterator xit = x.begin(); xit!=stop; ++xit) {
      value += *xit;
    }
  }

  void operator=( MaxSumOf& x ) {
    value = x.value;
  }

  bool operator>( MaxSumOf& x ) {
    return value > x.value;
  }

};

class MaxEuclideanSumOf {

public:

  MaxEuclideanSumOf() { clear(); }
  virtual ~MaxEuclideanSumOf() {}

  double value;
  
  void clear() { value = -INFTY; }

  void operator=( vector<double>& x) {
    value = 0;
    vector<double>::iterator stop = x.end();
    for( vector<double>::iterator xit = x.begin(); xit!=stop; ++xit) {
      value += (*xit) * (*xit);
    }
  }

  void operator=( MaxEuclideanSumOf& x ) {
    value = x.value;
  }

  bool operator>( MaxEuclideanSumOf& x ) {
    return value > x.value;
  }

};

class FirstLex {

public:

  FirstLex() {}
  virtual ~FirstLex() {}

  double value;
  vector<double> values;

  void clear() { values.clear(); }

  void operator=( vector<double>& x) {
    values.clear();
     vector<double>::iterator stop = x.end();
    for( vector<double>::iterator xit = x.begin(); xit!=stop; ++xit) {
      values.push_back(*xit);
    }
    sort(values.begin(), values.end());
  }

  bool operator=( FirstLex& x ) {
    //values.swap(x.values);
    values.clear();
    vector<double>::iterator stop = x.values.end();
    for( vector<double>::iterator xit = x.values.begin(); xit!=stop; ++xit) {
      values.push_back(*xit);
    }
  }

  bool operator>( FirstLex& x ) {
    vector<double>::iterator x_it = x.values.end();
    vector<double>::iterator x_end = x.values.begin();

    vector<double>::iterator me_it = values.end();
    vector<double>::iterator me_end = values.begin();

    int result = -1;
    do {
      if(me_it-- == me_end) result = 0;
      else if(x_it-- == x_end) result = 1;
      else if(*me_it > *x_it) result = 1;
      else if(*me_it < *x_it) result = 0;
    } while( result<0 );
    
    return result;
  }

};


class MaxCardFirstLex {

public:

  MaxCardFirstLex() {}
  virtual ~MaxCardFirstLex() {}

  double value;
  vector<double> values;

  void clear() { values.clear(); }

  void operator=( vector<double>& x) {
    values.clear();
    vector<double>::iterator stop = x.end();
    for( vector<double>::iterator xit = x.begin(); xit!=stop; ++xit) {
      values.push_back(*xit);
    }
    sort(values.begin(), values.end());
  }

  bool operator=( MaxCardFirstLex& x ) {
    //values.swap(x.values);
    values.clear();
    vector<double>::iterator stop = x.values.end();
    for( vector<double>::iterator xit = x.values.begin(); xit!=stop; ++xit) {
      values.push_back(*xit);
    }
  }

  bool operator>( MaxCardFirstLex& x ) {
    int result = -1;
    if(values.size() > x.values.size()) {
      result = 1;
    } else if(values.size() < x.values.size()) {
      result = 0;
    } else {


      vector<double>::iterator x_it = x.values.end();
      vector<double>::iterator x_end = x.values.begin();

      vector<double>::iterator me_it = values.end();
      vector<double>::iterator me_end = values.begin();
      
      do {
	if(me_it-- == me_end) result = 0;
	else if(x_it-- == x_end) result = 1;
	else if(*me_it > *x_it) result = 1;
	else if(*me_it < *x_it) result = 0;
      } while( result<0 );
    }      

    return result;
  }

};



template< class Method >
class Aggregation {
  
public:
  
  int randomization;
  int nbClasses;
  double *options_score;
  vector< int >::iterator *options_beg;
  vector< int >::iterator *options_end;
  vector<double> opt_score;
  Method               cur;
  Method              best;
  Method       second_best;

  Aggregation() { options_beg = options_end = NULL; randomization = 0; }
  //since choice= ValSelector(s,var.get_value_weight(),var.get_bound_weight()); in the GenericHeuristic class
  Aggregation(Solver *s, double **vw, double *bw,  const int r=0) { options_beg = options_end = NULL; randomization = r; } //options_beg = NULL;options_end = options_score = NULL; }
  Aggregation(Solver *s, const int r=0) { options_beg = options_end = NULL; randomization = r; } //options_beg = NULL;options_end = options_score = NULL; }
  virtual ~Aggregation() {
    delete [] options_beg;
    delete [] options_end;
  }

  virtual void initialise(CarSequencingInstance *instance, double *opt_weight, const int r=0) { //, double *cla_weight) {
    randomization = r;
    options_score = opt_weight;

    nbClasses = instance->nb_classes();
    options_beg = new vector< int >::iterator[nbClasses];
    options_end = new vector< int >::iterator[nbClasses];

    for(int cla=0; cla<nbClasses; ++cla) {
      options_beg[cla] = instance->obegin(cla);
      options_end[cla] = instance->oend(cla);
    }
  }

  void operator=( const Aggregation< Method >& ) {}

  std::ostream& display(std::ostream& os) const {
    os << "assign the class with best aggregated option's scores";
    return os;
  } 

  inline Decision make(Variable x) {
    Decision d(x, Decision::ASSIGNMENT, INFTY);
    Domain dom_x(x);
    Domain::iterator x_end = dom_x.end();
    Domain::iterator cla_it = dom_x.begin();
    int cla, best_class = 0, second_best_class=0;
    best.clear();
 
    for(; cla_it != x_end; ++cla_it) {
      cla = dom_x.get_value(cla_it);
      opt_score.clear();
      for(vector< int >::iterator opt_it = options_beg[cla]; opt_it != options_end[cla]; ++opt_it) {
	opt_score.push_back(options_score[*opt_it]);
      }

      
      cur = opt_score;

      //cout << "class_" << cla << ": " << cur.value ;

      if(cur > best) {

	//cout << "*";

	second_best = best;
	second_best_class = best_class;
	
	best = cur;
	best_class = cla;
      } else if(randomization && cur > second_best) {

	//cout << "'";

	second_best = cur;
	second_best_class = cla;
      }

      //cout << endl;
    }


    int rand = randint(CS_RAND_MAX);
    //cout << randomization  << " " << rand << endl;
    if(randomization && rand < randomization) {
      
      //cout << "use second best!" << endl;
      //cout << "(s)";

      best_class = second_best_class;
    }
    
    

    d.set_value(best_class);
    
    //cout << "  ==> " << d << endl << endl;
    return d;
    }

};
  

template< class Criterion >
class CarSequencingHeuristic : public SuccessListener {

public:
  CarSequencingModel *model;
  double *options_weight;

  Criterion  crit;

  CarSequencingHeuristic(CarSequencingModel *s) : model(s) {
    int nbOptions = model->instance->nb_options();
    options_weight = new double[nbOptions];
    crit.initialise(model->instance, options_weight);
    model->add((SuccessListener*)this);
  }

  virtual ~CarSequencingHeuristic() {
    model->remove((SuccessListener*)this);
  }

  virtual void notify_success() {
    crit.update(model);
  }
};


template< class VarSelector, class Method, class Criterion >
class ClassBranchingHeuristic : public CarSequencingHeuristic< Criterion >, public GenericHeuristic< VarSelector, Aggregation< Method > > {
  
public:
  ClassBranchingHeuristic(CarSequencingModel *s, const int r=0) : CarSequencingHeuristic< Criterion >(s), GenericHeuristic< VarSelector, Aggregation< Method > >(s) {
     GenericHeuristic< VarSelector, Aggregation< Method > >::choice.initialise(CarSequencingHeuristic< Criterion >::model->instance, 
									      CarSequencingHeuristic< Criterion >::options_weight, r); 
  }

  virtual ~ClassBranchingHeuristic() {
  }

};


template< class Criterion, class ValSelector >
class OptionByClassBranchingHeuristic : public CarSequencingHeuristic< Criterion >, public BranchingHeuristic, public VariableListener
{
  
public:
  
  Vector< Vector< Variable > >   order;
  Vector< Variable >           classes;
  ReversibleNum< int >            last;
  
  int nbOptions;
  int nbCars;
  int randomization;
  bool _lex_;

  
  ValSelector choice;


  virtual void notify_add_var() {};
  virtual void notify_change(const int idx) {

    // cout << "notify ";
    // cout << idx << " ";
    // cout << CarSequencingHeuristic< Criterion >::model->variables[idx] << " has changed!\n";
    
    // if(idx >= nbCars) {
      
    //   //cout << "OPTION!!" << endl;

    //   int car = CarSequencingHeuristic< Criterion >::model->get_car_index(idx);
    //   int opt = CarSequencingHeuristic< Criterion >::model->get_opt_index(idx);
      
    //   //cout << opt << " x " << car << endl;
      
  
      
      
    //   //cout << order[car][opt].id() << " == " << solver->variables[idx].id() << endl;

    //   order[car][opt] = solver->variables[idx];    
    // } else {
    if(idx < nbCars) {

      int car = idx;
      if(!_lex_) {
     	car -= nbCars/2;
     	car *= 2;
     	if(car<0) car = car*-1-1;
      }

      // cout << CarSequencingHeuristic< Criterion >::model->variables[idx] << " has changed! ?= " << classes[car] << endl << endl;



      // if(classes[car].id() != solver->variables[idx].id()) {
      // 	cerr << "ERROR!" << endl;
      // 	exit(1);
      // } 
      
      classes[car] = solver->variables[idx];

      //

    }
  }
  
  OptionByClassBranchingHeuristic(CarSequencingModel *s, const bool lex=true, const int r=0) : CarSequencingHeuristic< Criterion >(s), BranchingHeuristic(s) {
    nbCars = CarSequencingHeuristic< Criterion >::model->instance->nb_cars();
    nbOptions = CarSequencingHeuristic< Criterion >::model->instance->nb_options();
    randomization = r;
    _lex_ = lex;
    
    int car;
    
     order.initialise(nbCars, nbCars);
     classes.initialise(0, nbCars);

     for(int i=0; i<nbCars; ++i) {
       car = (lex ? i : nbCars/2 + (i%2 ? -1 : 1)*((i+1)/2));
       order[i].initialise(0, nbOptions);
       classes.add(CarSequencingHeuristic< Criterion >::model->class_at_position[car]);
       
       for(int opt=0; opt<nbOptions; ++opt) {
	 order[i].add(CarSequencingHeuristic< Criterion >::model->option[opt][car]);
       }

#ifdef _DEBUG_HEURISTIC
       cout << order[i] << endl;
#endif

     }

     last.initialise(s,0);     

     CarSequencingHeuristic< Criterion >::model->add((VariableListener*)this);
   }
  virtual ~OptionByClassBranchingHeuristic() {}
   
   virtual Decision branch() {
     return choice.make(select());
   }

   Variable select() {

#ifdef _PRINT_CLASSES
     CarSequencingModel *m = CarSequencingHeuristic< Criterion >::model;
     int nbClasses = m->instance->nb_classes();
     

     cout << endl;
     for(int j=0; j<nbClasses; ++j) {
       for(int i=0; i<nbCars; ++i) {
	 if(m->bool_class[j][i].is_ground()) cout << m->bool_class[j][i].get_min();
	 else cout << ".";
       }
       cout << endl;
     }
     cout << endl;
#endif



     int opt, best_opt=-1, second_best_opt=-1;

#ifdef _DEBUG_HEURISTIC
     cout << "find first unassigned slot: " << classes[last].get_domain() << (classes[last].is_ground() ? "-" : "+");
#endif

     while(last<nbCars && classes[last].is_ground()) { 
       
#ifdef _DEBUG_HEURISTIC
       cout << " " << classes[last] ;
#endif

       ++last;

#ifdef _DEBUG_HEURISTIC
       if(last < nbCars)
	 cout << " " << classes[last].get_domain() << (classes[last].is_ground() ? "-" : "+");
#endif
     }

#ifdef _DEBUG_HEURISTIC
     cout << " (" << last << ")";
     // cout.flush();
     cout << " go with " << classes[last] << endl << " rank options:\n";
#endif

     for(opt=0; opt<nbOptions; ++opt) {

#ifdef _DEBUG_HEURISTIC
       cout << " -option_" << opt << ": " ;
#endif

       if(!order[last][opt].is_ground()) {
	 
#ifdef _DEBUG_HEURISTIC
	 cout << CarSequencingHeuristic< Criterion >::options_weight[opt] ;
#endif

	 if(best_opt < 0 || CarSequencingHeuristic< Criterion >::options_weight[opt] > CarSequencingHeuristic< Criterion >::options_weight[best_opt]) {
	   second_best_opt = best_opt;
	   best_opt = opt;

#ifdef _DEBUG_HEURISTIC
	   cout << " best so far!\n";
#endif

	 } else if(randomization && (second_best_opt < 0 || CarSequencingHeuristic< Criterion >::options_weight[opt] > 
				     CarSequencingHeuristic< Criterion >::options_weight[second_best_opt])) {
	   second_best_opt = opt;

#ifdef _DEBUG_HEURISTIC
	   cout << " second best so far!\n";
#endif

	 }
       }

#ifdef _DEBUG_HEURISTIC
       else cout << "ground!\n";
#endif

     }

     if(randomization && second_best_opt>=0 && randint(CS_RAND_MAX) < randomization) {
       best_opt = second_best_opt;
     }

#ifdef _DEBUG_HEURISTIC     
     cout << last << " " << best_opt << endl;
     cout << "return " << order[last][best_opt] << " in " << order[last][best_opt].get_domain() << endl;
#endif

     return order[last][best_opt];
   }
   
  virtual std::ostream& display(std::ostream& os)  {
    os << "Go by lexicographic order: " ;
    
    int i = last;
    while(i<(int)(order.size) && classes[i].is_ground()) { 
      ++i;
    }
    os << order[i];
    return os;
  }
  

 };






template< class Criterion, class ValSelector >
class OptionBranchingHeuristic : public CarSequencingHeuristic< Criterion >, public BranchingHeuristic {
  
public:
  
  Vector< Vector< Variable > >   order;
  Vector< ReversibleNum< int > >  last;
  
  int nbOptions;
  int nbCars;
  int randomization;
  
  ValSelector choice;
  
  OptionBranchingHeuristic(CarSequencingModel *s, const bool lex=true, const int r=0) : CarSequencingHeuristic< Criterion >(s), BranchingHeuristic(s) {
    nbCars = CarSequencingHeuristic< Criterion >::model->instance->nb_cars();
    nbOptions = CarSequencingHeuristic< Criterion >::model->instance->nb_options();
    randomization = r;
    
    int car;
    
     order.initialise(nbOptions, nbOptions);
     last.initialise(0, nbOptions);

     for(int opt=0; opt<nbOptions; ++opt) {
       order[opt].initialise(0, nbCars);
       last[opt].initialise(s,0);       
       
       for(int i=0; i<nbCars; ++i) {
	 car = (lex ? i : nbCars/2 + (i%2 ? -1 : 1)*(i/2+1));
	 order[opt].add(CarSequencingHeuristic< Criterion >::model->option[opt][car]);
       }

#ifdef _DEBUG_HEURISTIC
       cout << order[opt] << endl;
#endif

     }
   }
  virtual ~OptionBranchingHeuristic() {}
   
   virtual Decision branch() {
     return choice.make(select());
   }

   Variable select() {
     int opt, best_opt=-1, second_best_opt=-1;

#ifdef _DEBUG_HEURISTIC
     cout << "rank options:\n" ;
#endif

     // find the best option
     for(opt=0; opt<nbOptions; ++opt) {
       
#ifdef _DEBUG_HEURISTIC
       cout << " -option_" << opt << ": " ;
       cout << CarSequencingHeuristic< Criterion >::options_weight[opt] ;
#endif
	 
       if(best_opt < 0 || CarSequencingHeuristic< Criterion >::options_weight[opt] > CarSequencingHeuristic< Criterion >::options_weight[best_opt]) {
	 second_best_opt = best_opt;
	 best_opt = opt;

#ifdef _DEBUG_HEURISTIC	 
	 cout << " best so far!\n";
#endif

       } else if(randomization && (second_best_opt < 0 || CarSequencingHeuristic< Criterion >::options_weight[opt] > 
				   CarSequencingHeuristic< Criterion >::options_weight[second_best_opt])) {
	 second_best_opt = opt;
	 
#ifdef _DEBUG_HEURISTIC
	 cout << " second best so far!\n";
#endif

       }
     }

     if(randomization && second_best_opt>=0 && randint(CS_RAND_MAX) < randomization) {
       best_opt = second_best_opt;
     }

#ifdef _DEBUG_HEURISTIC
     cout << "find first unassigned slot:";
#endif

     while(last[best_opt]<nbCars && order[best_opt][last[best_opt]].is_ground()) { 
       
#ifdef _DEBUG_HEURISTIC
       cout << " [" << last[best_opt] << "]=" << order[best_opt][last[best_opt]].get_domain() ;
#endif

       ++last[best_opt];
     }

#ifdef _DEBUG_HEURISTIC
     cout << " (" << last[best_opt] << ")";
     // cout.flush();
     cout << " go with " << order[best_opt][last[best_opt]] << endl << endl;
#endif
     
     return order[best_opt][last[best_opt]];
   }
   
  virtual std::ostream& display(std::ostream& os)  {
    os << "Go by lexicographic order: " ;
    
    // int i = last;
    // while(i<(int)(order.size) && classes[i].is_ground()) { 
    //   ++i;
    // }
    // os << order[i];
    return os;
  }
  

 };


// template< class VarSelector, class Criterion >
// class OptionBranchingHeuristic : public GenericHeuristic< VarSelector, MaxWeight >, public SuccessListener {
  
// public:
  
//   CarSequencingModel *model;
//   double *options_weight;

//   Criterion  crit;
  
//   OptionBranchingHeuristic(CarSequencingModel *s, const int r=0) : GenericHeuristic< VarSelector, Aggregation< Method > >(s), model(s) {
//     int nbOptions = model->instance->nb_options();
//     options_weight = new double[nbOptions];
    
//     crit.initialise(model->instance, options_weight);
//     GenericHeuristic< ,  >::choice.initialise(options_weight); 
    
//     model->add((SuccessListener*)this);
//   }

//   virtual ~OptionBranchingHeuristic() {
//     model->remove((SuccessListener*)this);
//   }

//   virtual void notify_success() {
//     crit.update(model);
//   }

// };




CarSequencingModel *modelFactory(string model, CarSequencingInstance *instance, bool rewrite) {
  CarSequencingModel *solver;
  if(model == "sum") {
    solver = new SumModel(instance);
  } else if(model == "amsc+sum") {
    solver = new SumAmscModel(instance);
  } else if(model == "mamsc+sum") {
    solver = new SumMultiAmscModel(instance);
  } else {
    throw ArgException(string(" "),
		       string("model ")+
		       model+
		       string(" is not implemented yet"),
		       string("model"));
  }

  solver->setup();
  if(rewrite) solver->rewrite();
  solver->consolidate();
  for(int i=0; i<solver->instance->nb_cars(); ++i) {
    solver->class_at_position[i] = solver->class_at_position[i].get_var();
    for(int opt=0; opt<solver->instance->nb_options(); ++opt) {
      solver->option[opt][i] = solver->option[opt][i].get_var();
    }
  }

  return solver;
}

BranchingHeuristic *heuristicFactory(CarSequencingModel *solver, 
				     string branching, string exploration, string criterion, string aggregation, int randomization) {
  BranchingHeuristic *heuristic = NULL;

  if(branching == "class") {
    if(criterion == "demand") {
      if(aggregation == "lex") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, FirstLex, Dynamic<Demand> >(solver, randomization);
      } else if(aggregation == "sum") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxSumOf, Dynamic<Demand> >(solver, randomization);
      } else if(aggregation == "euc") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxEuclideanSumOf, Dynamic<Demand> >(solver, randomization);
      } else if(aggregation == "card+lex") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxCardFirstLex, Dynamic<Demand> >(solver, randomization);
      }
    } else if(criterion == "pq") {
      if(aggregation == "lex") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, FirstLex, StaticCapacity >(solver, randomization);
      } else if(aggregation == "sum") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxSumOf, StaticCapacity >(solver, randomization);
      } else if(aggregation == "euc") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxEuclideanSumOf, StaticCapacity >(solver, randomization);
      } else if(aggregation == "card+lex") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxCardFirstLex, StaticCapacity >(solver, randomization);
      }
    } else if(criterion == "load") {
      if(aggregation == "lex") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, FirstLex, Dynamic<Load> >(solver, randomization);
      } else if(aggregation == "sum") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxSumOf, Dynamic<Load> >(solver, randomization);
      } else if(aggregation == "euc") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxEuclideanSumOf, Dynamic<Load> >(solver, randomization);
      } else if(aggregation == "card+lex") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxCardFirstLex, Dynamic<Load> >(solver, randomization);
      }
    } else if(criterion == "rate") {
      if(aggregation == "lex") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, FirstLex, Dynamic<Rate> >(solver, randomization);
      } else if(aggregation == "sum") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxSumOf, Dynamic<Rate> >(solver, randomization);
      } else if(aggregation == "euc") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxEuclideanSumOf, Dynamic<Rate> >(solver, randomization);
      } else if(aggregation == "card+lex") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxCardFirstLex, Dynamic<Rate> >(solver, randomization);
      }
    } else if(criterion == "slack") {
      if(aggregation == "lex") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, FirstLex, Dynamic<Slack> >(solver, randomization);
      } else if(aggregation == "sum") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxSumOf, Dynamic<Slack> >(solver, randomization);
      } else if(aggregation == "euc") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxEuclideanSumOf, Dynamic<Slack> >(solver, randomization);
      } else if(aggregation == "card+lex") {
	heuristic = new ClassBranchingHeuristic < Lexicographic, MaxCardFirstLex, Dynamic<Slack> >(solver, randomization);
      }
    }
  } else if(branching == "option") {
    if(criterion == "demand") {
      heuristic = new OptionBranchingHeuristic < Dynamic<Demand>, MaxValue >(solver, (exploration == "lex"), randomization);
    } else if(criterion == "pq") {
      heuristic = new OptionBranchingHeuristic < StaticCapacity, MaxValue >(solver, (exploration == "lex"), randomization);
    } else if(criterion == "load") {
      heuristic = new OptionBranchingHeuristic < Dynamic<Load>, MaxValue >(solver, (exploration == "lex"), randomization);
    } else if(criterion == "rate") {
      heuristic = new OptionBranchingHeuristic < Dynamic<Rate>, MaxValue >(solver, (exploration == "lex"), randomization);
    } else if(criterion == "slack") {
      heuristic = new OptionBranchingHeuristic < Dynamic<Slack>, MaxValue >(solver, (exploration == "lex"), randomization);
    } 
  } else if(branching == "slot") {
    if(criterion == "demand") {
      heuristic = new OptionByClassBranchingHeuristic < Dynamic<Demand>, MaxValue >(solver, (exploration == "lex"), randomization);
    } else if(criterion == "pq") {
      heuristic = new OptionByClassBranchingHeuristic < StaticCapacity, MaxValue >(solver, (exploration == "lex"), randomization);
    } else if(criterion == "load") {
      heuristic = new OptionByClassBranchingHeuristic < Dynamic<Load>, MaxValue >(solver, (exploration == "lex"), randomization);
    } else if(criterion == "rate") {
      heuristic = new OptionByClassBranchingHeuristic < Dynamic<Rate>, MaxValue >(solver, (exploration == "lex"), randomization);
    } else if(criterion == "slack") {
      heuristic = new OptionByClassBranchingHeuristic < Dynamic<Slack>, MaxValue >(solver, (exploration == "lex"), randomization);
    }
  }
   
  if(!heuristic) {
    std::cerr << "Error: unknown heuristic <" << branching << " x " << aggregation << " x " << criterion << ">\n";
    exit(0);
  }

 return heuristic;
}

RestartPolicy *restartFactory(string restart) {
  RestartPolicy *restartp;
  if(restart == "geom") {
    restartp = new Geometric();
  } else if(restart == "luby") {
    restartp = new Luby();
  } else {
    restartp = new NoRestart();
  }

  return restartp;
}


int main(int argc, char **argv)
{
  try 
    {  


      // Define the command line object.
      CmdLine cmd("car_sequencing", ' ', "0.0");
      
      ValueArg<string> fileArg("f","file","instance file",true,"data/carseq/easy/60_01","string");
      cmd.add( fileArg );

      vector<string> mallowed;
      mallowed.push_back("sum");
      mallowed.push_back("amsc+sum");
      mallowed.push_back("gsc");
      mallowed.push_back("amsc");
      mallowed.push_back("mamsc+sum");
      ValuesConstraint<string> m_allowed( mallowed );
      ValueArg<string> modelArg("m","model","type of model",false,"sum",&m_allowed); //"amsc","string");
      cmd.add( modelArg );

      vector<string> ballowed;
      ballowed.push_back("slot");
      ballowed.push_back("class");
      ballowed.push_back("option");
      ValuesConstraint<string> b_allowed( ballowed );
      ValueArg<string> branchingArg("b","branch","type of branching",false,"class",&b_allowed);
      cmd.add( branchingArg );
      
      vector<string> eallowed;
      eallowed.push_back("lex");
      eallowed.push_back("middle");
      ValuesConstraint<string> e_allowed( eallowed );
      ValueArg<string> explorationArg("e","explore","type of exploration",false,"lex",&e_allowed);
      cmd.add( explorationArg );

      vector<string> callowed;
      //callowed.push_back("id");
      callowed.push_back("demand");
      callowed.push_back("pq");
      callowed.push_back("load");
      callowed.push_back("rate");
      callowed.push_back("slack");
      ValuesConstraint<string> c_allowed( callowed );
      ValueArg<string> criterionArg("c","criterion","option criterion",false,"demand",&c_allowed);
      cmd.add( criterionArg );

      vector<string> aallowed;
      aallowed.push_back("sum");
      aallowed.push_back("euc");
      //aallowed.push_back("max");
      aallowed.push_back("lex");
      aallowed.push_back("card+lex");
      ValuesConstraint<string> a_allowed( aallowed );
      ValueArg<string> aggregationArg("a","aggregation","aggregation method",false,"sum",&a_allowed);
      cmd.add( aggregationArg );
      
      vector<string> rallowed;
      rallowed.push_back("no");
      rallowed.push_back("geom");
      rallowed.push_back("luby");
      ValuesConstraint<string> r_allowed( rallowed );
      ValueArg<string> restartArg("r","restart","restart method",false,"no",&r_allowed);
      cmd.add( restartArg );
      
      ValueArg<double> randomizationArg("d","randomization","randomization level",false,0,"int");
      cmd.add( randomizationArg );

      ValueArg<int> seedArg("s","seed","random seed",false,12345,"int");
      cmd.add( seedArg );

      ValueArg<int> verbosityArg("v","verbosity","verbosity level",false,1,"int");
      cmd.add( verbosityArg );

      SwitchArg rewriteSwitch("w","rewrite","Use rewriting during preprocessing", false);
      cmd.add( rewriteSwitch );

      vector<string> pallowed;
      pallowed.push_back("params");
      pallowed.push_back("instance");
      pallowed.push_back("model");
      pallowed.push_back("sol");
      pallowed.push_back("stat");
      ValuesConstraint<string> p_allowed( pallowed );
      MultiArg<string> printArg("p","print","Objects to print, in {params, instance, model, sol, stat}.",false,&p_allowed);
      cmd.add( printArg );
      

      // Parse the args.
      cmd.parse( argc, argv );
    

      // Get the value parsed by each arg. 
      string file = fileArg.getValue();
      string model = modelArg.getValue();
      string branching = branchingArg.getValue();
      string exploration = explorationArg.getValue();
      string criterion = criterionArg.getValue();
      string aggregation = aggregationArg.getValue();
      string restart = restartArg.getValue();

      int randomization = (int)((double)CS_RAND_MAX * randomizationArg.getValue());


      int seed = seedArg.getValue();
      int verbosity = verbosityArg.getValue();
      
      bool rewrite = rewriteSwitch.getValue();

      vector<string> print = printArg.getValue();
      map< string, bool > should_print;
      should_print[string("params")] = false;
      should_print[string("instance")] = false;
      should_print[string("model")] = false;
      should_print[string("sol")] = false;
      should_print[string("stat")] = false;
      
      vector<string>::iterator to_print = print.begin();
      vector<string>::iterator stop = print.end();
      while(to_print != stop) {
	should_print[*to_print] = true;
	++to_print;
      }

      // if(should_print[string("params")]) {

      // }

      usrand(seed);

      
      CarSequencingInstance instance(file.c_str());

      if(should_print[string("instance")]) {
	cout << instance << endl;
      }

      CarSequencingModel *solver = modelFactory(model, &instance, rewrite);

      if(should_print[string("model")]) {
	cout << (*solver) << endl;
      }

      BranchingHeuristic *heuristic = heuristicFactory(solver, branching, exploration, criterion, aggregation, randomization);
      
      solver->parameters.verbosity = verbosity;
      
#ifdef _MONITOR
      for(int opt=0; opt<instance.nb_options(); ++opt) {
	solver->monitor_list << solver->option[opt];
	solver->monitor_list << "\n";
      }
#endif
      
      RestartPolicy *restartp = restartFactory(restart);
      
      
      
      Outcome result = solver->depth_first_search(solver->class_at_position, heuristic, restartp);
      
      
      if(result) {
	cout << "SATISFIABLE!" << endl;
	if(should_print[string("sol")]) {
	  Solution sol(solver->class_at_position);
	  cout << " c  solution: " << sol << endl;
	}
      } else {
	cout << "UNSATISFIABLE!" << endl;
      }
      
      if(should_print[string("stat")]) {
	cout << solver->statistics << endl;
      }

      delete solver;
      
    } 
  catch (ArgException &e)  // catch any exceptions 
    { 
      cerr << "error: " << e.error() << " for arg " << e.argId() << endl; 
    }
  
}



