#include <iostream>
#include <string>
#include <type_traits>

using namespace std;


//-----------------------  Solution 1 (fun with func pointer to template func!) --------------------------//

template<typename T>
void prpr(void* ptr) {
	std::cout << *(static_cast<T*>(ptr));
}

class Printer {

public:
	Printer() : p_value(nullptr), my_ptr(false), PrintFunc(nullptr) {};
	~Printer() { if(my_ptr) delete p_value; };

	void print() const {
		if (p_value && PrintFunc) {
			std::cout << (my_ptr ? "" : "[");
			PrintFunc(p_value);
			std::cout << (my_ptr ? "" : "]") << std::endl;
		}
	}; 
	
    template<typename T>
    void new_value(T&& x) { p_value = new T(x); my_ptr = true; PrintFunc = &prpr<T>;}
		
    template<typename T>
    void new_value(T* x) { p_value = x; my_ptr = false; PrintFunc = &prpr<T>;}
    
    template<typename T>
    void new_value(T& x) { p_value = x; my_ptr = false; PrintFunc = &prpr<T>;}

    template<typename T>
    Printer& operator=(T&& x) noexcept {
	if(my_ptr)
	    delete p_value;
	new_value(std::forward<T>(x));
	return *this; 
     };
	

private:
	void* p_value;
	bool my_ptr;
	void(*PrintFunc)(void*);
};


//-----------------------  Solution 2 (boring polymorphic, but right) --------------------------//

class A {
public:
	virtual ~A() = default;
	virtual void print() const = 0;
};

template<typename T>
class B : public A {
public:
	B(T value) : value_(value) {};
	virtual void print() const {
		std::cout << value_ << std::endl;
	};
	T value_;
};

template<typename T>
class B_ptr : public A {
public:
	B_ptr(T* value) : value_(value) {};
	virtual void print() const {
		std::cout << "[" << *value_ << "]" << std::endl;
	};
	T* value_;
};


class Printer2 {
public:
	Printer2() : value(nullptr) {};

	void print() const {
		value->print();
	};

    template<typename T>
    void new_value(T&& x) { value = new B<T>(x);}
		
    template<typename T>
    void new_value(T* x) { value = new B_ptr<T>(x);}
    
    template<typename T>
    void new_value(T& x) { value = new B_ptr<T>(x);}

    template<typename T>
    Printer2& operator=(T&& x) noexcept {
    	delete value;
	new_value(std::forward<T>(x));
	return *this;
     };

private:
	A* value;
};


//-----------------------  Solution 3 (shorter version of solution 2) --------------------------//
//-----------  No need in surplus pointer handling stuff when you have type traits -------------//

class A3 {
public:
	virtual ~A3() = default;
	virtual void print() const = 0;
};

template<typename T>
class B3 : public A3 {
public:
	B3(T value) : value_(value) {};
	virtual void print() const {
		std::cout << value_ << std::endl;
	};
	T value_;
};

class Printer3 {
public:
	Printer3() : value(nullptr) {};
	~Printer3() { delete value; };

	void print() const {
		value->print();
	};
		
    template<typename T>
    Printer3& operator=(T&& x) noexcept {
        delete value;
	if constexpr(std::is_pointer<T>::value || std::is_lvalue_reference<T>::value) {
            value = new B3<decltype(*x)>(*x);
        }
        else
            value = new B3<T>(x);
	return *this;
    }

private:
	A3* value;
};



//-------------------------------------------------------------//



int main() {
	Printer3 printer;

	printer = 42;
	printer.print(); //should print "42" to standard output

	int* value = new int(10);
	printer = value;
	printer.print(); // should print "[10]"

	*value = 20; // the value pointer did not change, changed a value by it
	printer.print(); // should print "[20]"

	float* fvalue = new float(9.81);
	printer = fvalue;
	printer.print(); // should print "[9.81]"

	*fvalue = 0.2 + 0.3;
	printer.print(); // should print "[0.5]"

	printer = std::string("Hello world");
	printer.print();
	//should print "Hello world"

	printer = 2.718281;
	printer.print();
	//should print "2.718281"

	delete fvalue;
	delete value;
	return 0;
}
