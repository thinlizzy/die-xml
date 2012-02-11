#ifndef OPTIONAL_H_jjj54444409vgfdgkj234
#define OPTIONAL_H_jjj54444409vgfdgkj234

#include <memory>

template<typename T>
class FreeStorePolicy {
	std::unique_ptr<T> data;
public:
	FreeStorePolicy() {}
	explicit FreeStorePolicy(T const & value): data(new T(value)) {}
	FreeStorePolicy(FreeStorePolicy const & policy): data( policy.data ? new T(*policy.data) : 0 ) {}
	FreeStorePolicy & operator=(FreeStorePolicy const & policy) {
		swap(FreeStorePolicy(policy));
		return *this;
	}
	void swap(FreeStorePolicy && other) throw() { data.swap(other.data); }
	void setValue(T const & value) {
		if( data ) {
			*data = value;
		} else {
			data.reset(new T(value));
		}
	}
	T & operator*() { return *data; }
	T const & operator*() const { return *data; }
	T * operator->() { return data.get(); }
	T const * operator->() const { return data.get(); }
};

template<typename T>
class AutomaticStoragePolicy {
	bool hasValue;
	T data;
public:
	AutomaticStoragePolicy(): hasValue(false) {}
	explicit AutomaticStoragePolicy(T const & value): hasValue(true), data(value) {}
	void swap(AutomaticStoragePolicy && other) throw() { 
		std::swap(hasValue,other.hasValue); 
		std::swap(data,std::move(other.data)); 
	}
	void setValue(T const & value) {
		hasValue = true;
		data = value;
	}
	T & operator*() { return data; }
	T const & operator*() const { return data; }
	T * operator->() { return hasValue ? &data : 0; }
	T const * operator->() const { return hasValue ? &data : 0; }
};

// TODO make conversion operators

//* TODO VS ISSUE waiting for a new VS version
template<typename T, bool big>
struct StoragePolicyTraits {
	typedef FreeStorePolicy<T> type;
};

template<typename T>
struct StoragePolicyTraits<T,false> {
	typedef AutomaticStoragePolicy<T> type;
};

template<typename T, typename StoragePolicy = StoragePolicyTraits<T,(sizeof(T) > sizeof(T*))>::type>
class optional {
	StoragePolicy storage;

/*/
template<typename T, bool big>
struct StoragePolicyTraits {
	//template <class T> using type = FreeStorePolicy<T>;
	template <typename U = T> struct type : FreeStorePolicy<U> { };
};



template<typename T>
struct StoragePolicyTraits<T,false> {
	//template <class T> using type = AutomaticStoragePolicy<T>;
	template <typename U = T> struct type : AutomaticStoragePolicy<U> { };
};

template<typename T, template<typename> class StoragePolicy = StoragePolicyTraits<T,(sizeof(T) > sizeof(T*))>::template type>  // VS ISSUE
class optional {
	StoragePolicy<T> storage;
//*/
public:
	optional() {}

	optional(T const & value):
		storage(value)
	{
	}

	void swap(optional && opt) throw()
	{
		storage.swap(std::move(opt.storage));
	}

	optional & operator=(T const & value) 
	{
		storage.setValue(value);
		return *this;
	}

	T & operator*() { return *storage; }
	T const & operator*() const { return *storage; }
	T * operator->() { return storage.operator->(); }
	T const * operator->() const { return storage.operator->(); }

	operator void const * () const { return operator->(); }
	bool operator!() const { return operator->() == 0; }
};

#endif
