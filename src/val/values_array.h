#ifndef VAL_VALUES_ARRAY_H
#define VAL_VALUES_ARRAY_H

#include <typeinfo>

#include "settings.h"

namespace val {
  
  // Array values

  // here we need a forward declaration
  template <typename T> class ArrayValue; 
  template <typename T> class BaseArrayValue; //: public BaseValue;
  template <> class ArrayValue<std::string>; //: public BaseArrayValue<bool>;
  template <> class ArrayValue<bool>; //: public BaseArrayValue<bool>;
  
  template <typename T>
  class BaseArrayValue: public BaseValue {
  protected:
    std::vector<T> value;
    Array::ShapeType shape;
  public:
    BaseArrayValue(const T& val, const Array::ShapeType& sh, const ValueDtype dt): value({val}), shape(sh), BaseValue(dt) {};
    BaseArrayValue(const std::vector<T>&  arr, const Array::ShapeType& sh, const ValueDtype dt): value(arr), shape(sh), BaseValue(dt) {};
    void print() override {std::cout << to_string() << std::endl;};
    std::vector<T> get_values() const {return value;};
    T get_value(const size_t index) const {return value.at(index);};
    Array::ShapeType get_shape() const override {return shape;};
    size_t get_size() const override {return value.size();};
  protected:
    virtual void value_to_string(std::ostringstream& oss, size_t& offset, int precision=0) const = 0;
    std::string to_string_dim(size_t& offset, const int& precision=0, int dim=0) const {
      std::ostringstream oss;
      oss << "[";
      for (int i=0; i<shape[dim];i++) {
	if (i>0) oss << ", ";
	if (dim+1<shape.size()) {
	  oss << to_string_dim(offset, precision, dim+1);
	} else {
	  value_to_string(oss, offset, precision);
	  offset++;
	}
      }
      oss << "]";
      return oss.str();
    }
  public:
    std::string to_string(const int precision=0) const override {
      size_t offset = 0;
      if (precision==0) {
	return to_string_dim(offset);
      } else {
	return to_string_dim(offset, precision);
      }
    };
    bool operator==(const BaseValue* other) const override {
      const BaseArrayValue<T>* otherT = dynamic_cast<const BaseArrayValue<T>*>(other);
      if (otherT) {
	bool is_equal = true;
	for (int i=0; i<value.size();i++)
	  if (value[i]!=otherT->value[i])
	    is_equal = false;
	for (int i=0; i<shape.size();i++)
	  if (shape[i]!=otherT->shape[i])
	    is_equal = false;	
	return is_equal;
      } else {
	throw std::runtime_error("Could not convert BaseValue into a BaseArrayValue");
      }
    };
    bool operator<(const BaseValue* other) const override {
      const BaseArrayValue<T>* otherT = dynamic_cast<const BaseArrayValue<T>*>(other);
      if (otherT) {
	bool is_equal = true;
	for (int i=0; i<value.size();i++)
	  if (value[i]>=otherT->value[i])
	    is_equal = false;
	for (int i=0; i<shape.size();i++)
	  if (shape[i]>=otherT->shape[i])
	    is_equal = false;	
	return is_equal;
      } else {
	throw std::runtime_error("Could not convert BaseValue into a BaseArrayValue");
      }
    };
    BaseValue::PointerType slice_value(const Array::RangeType& slice) {
      if (slice.size()!=this->shape.size())
	throw std::runtime_error("Array slice size does not correspond with array shape: "+std::to_string(slice.size())+"!="+std::to_string(this->shape.size()));
      // calculate new shape and size
      Array::ShapeType new_shape;
      size_t new_size = 0;
      for (size_t i=0; i<this->shape.size(); i++) {
	int dmin = slice[i].dmin;
	int dmax = (slice[i].dmax==Array::max_range) ? this->shape[i]-1 : slice[i].dmax;
	int new_dim = dmax + 1 - dmin;
	if (new_dim>1) {
	  new_shape.push_back(new_dim);
	  new_size += new_dim;
	}
      }
      // allocate variables
      std::vector<T> new_value;
      new_value.reserve(new_size);
      Array::ShapeType coord(this->shape.size(),0);
      // list through all values and copy sliced
      for (size_t i=0; i<this->value.size(); i++) {
	// copy only values within the given ranges
	bool push = true;
	for (size_t dim = 0; dim<this->shape.size(); dim++) {
	  int dmin = slice[dim].dmin;
	  int dmax = (slice[dim].dmax==Array::max_range) ? this->shape[dim]-1 : slice[dim].dmax;
	  if (coord[dim]<dmin or dmax<coord[dim])
	    push = false;
	}
	if (push)
	  new_value.push_back(this->value[i]);
	// increase the coodrinates
	for (size_t dim = this->shape.size(); dim-->0;) {
	  if (++coord[dim] < this->shape[dim]) break;
	  coord[dim] = 0;  // carry over
	}
      }
      if (new_size>1)
	return std::make_unique<ArrayValue<T>>(new_value, new_shape, this->dtype);
      else
	return std::make_unique<ArrayValue<T>>(new_value[0], Array::ShapeType({1}), this->dtype);
    };
    void convert_units(const std::string& from_units, const Quantity::PointerType& to_quantity) override {
      throw std::runtime_error("Array value of type '"+std::string(ValueDtypeNames[dtype])+"' does not support unit conversion.");
    };
    void convert_units(const Quantity::PointerType& from_quantity, const std::string& to_units) override {
      throw std::runtime_error("Array value of type '"+std::string(ValueDtypeNames[dtype])+"' does not support unit conversion.");
    };
  };
  
  template <typename T>
  class ArrayValue: public BaseArrayValue<T> {
  public:
    ArrayValue(const T& val, const Array::ShapeType& sh, const ValueDtype dt): BaseArrayValue<T>(val,sh,dt) {};
    ArrayValue(const std::vector<T>&  arr, const Array::ShapeType& sh, const ValueDtype dt): BaseArrayValue<T>(arr,sh,dt) {};
  private:
    void value_to_string(std::ostringstream& oss, size_t& offset, int precision=0) const override {
      if (precision==0) precision=DISPLAY_FLOAT_PRECISION;
      int exponent = static_cast<int>(std::log10(std::fabs(this->value[offset])));
      if (exponent > 3 || exponent < -3) {
        oss << std::scientific << std::setprecision(precision) << this->value[offset];
      } else {
        oss << std::fixed << std::setprecision(precision-exponent) << this->value[offset];
      }
    };
  public:
    BaseValue::PointerType clone() const override {
      return std::make_unique<ArrayValue<T>>(this->value, this->shape, this->dtype);
    };
    BaseValue::PointerType cast_as(ValueDtype dt) const override {
      switch (dt) {
      case ValueDtype::Boolean: {
	std::vector<bool> arr(this->value.size());
	for (size_t i=0; i<this->value.size(); i++)
	  arr[i] = static_cast<bool>(this->value[i]);
	return std::make_unique<ArrayValue<bool>>(arr, this->shape, dt);
      }
      case ValueDtype::Integer16: {
	std::vector<int16_t> arr(this->value.size());
	for (size_t i=0; i<this->value.size(); i++)
	  arr[i] = static_cast<int16_t>(this->value[i]);
	return std::make_unique<ArrayValue<int16_t>>(arr, this->shape, dt);
      }
      case ValueDtype::Integer32: {
	std::vector<int32_t> arr(this->value.size());
	for (size_t i=0; i<this->value.size(); i++)
	  arr[i] = static_cast<int32_t>(this->value[i]);
	return std::make_unique<ArrayValue<int32_t>>(arr, this->shape, dt);
      }
      case ValueDtype::Integer64: {
	std::vector<int64_t> arr(this->value.size());
	for (size_t i=0; i<this->value.size(); i++)
	  arr[i] = static_cast<int64_t>(this->value[i]);
	return std::make_unique<ArrayValue<int64_t>>(arr, this->shape, dt);
      }
      case ValueDtype::Integer16_U: {
	std::vector<uint16_t> arr(this->value.size());
	for (size_t i=0; i<this->value.size(); i++)
	  arr[i] = static_cast<uint16_t>(this->value[i]);
	return std::make_unique<ArrayValue<uint16_t>>(arr, this->shape, dt);
      }
      case ValueDtype::Integer32_U: {
	std::vector<uint32_t> arr(this->value.size());
	for (size_t i=0; i<this->value.size(); i++)
	  arr[i] = static_cast<uint32_t>(this->value[i]);
	return std::make_unique<ArrayValue<uint32_t>>(arr, this->shape, dt);
      }
      case ValueDtype::Integer64_U: {
	std::vector<uint64_t> arr(this->value.size());
	for (size_t i=0; i<this->value.size(); i++)
	  arr[i] = static_cast<uint64_t>(this->value[i]);
	return std::make_unique<ArrayValue<uint64_t>>(arr, this->shape, dt);
      }
      case ValueDtype::Float32: {
	std::vector<float> arr(this->value.size());
	for (size_t i=0; i<this->value.size(); i++)
	  arr[i] = static_cast<float>(this->value[i]);
	return std::make_unique<ArrayValue<float>>(arr, this->shape, dt);
      }
      case ValueDtype::Float64: {
	std::vector<double> arr(this->value.size());
	for (size_t i=0; i<this->value.size(); i++)
	  arr[i] = static_cast<double>(this->value[i]);
	return std::make_unique<ArrayValue<double>>(arr, this->shape, dt);
      }
      case ValueDtype::Float128: {
	std::vector<long double> arr(this->value.size());
	for (size_t i=0; i<this->value.size(); i++)
	  arr[i] = static_cast<long double>(this->value[i]);
	return std::make_unique<ArrayValue<long double>>(arr, this->shape, dt);
      }
      case ValueDtype::String: {
	std::vector<std::string> arr(this->value.size());
	for (size_t i=0; i<this->value.size(); i++)
	  arr[i] = std::to_string(this->value[i]);
	return std::make_unique<ArrayValue<std::string>>(arr, this->shape, dt);
	}
      default:
	throw std::runtime_error("Not implemented");
	return nullptr;
      }
    };
    BaseValue::PointerType slice(const Array::RangeType& slice) override {
      return this->slice_value(slice);
    };
    void convert_units(const std::string& from_units, const Quantity::PointerType& to_quantity) override {
      // TODO: use the same BaseValue pointers in the puq to allow variable precision
      std::vector<double> input(this->value.begin(), this->value.end());
      puq::Quantity quantity = puq::Quantity(input, from_units);
      quantity = quantity.convert(*to_quantity);
      std::vector<double> output = quantity.value.magnitude.value.value;
      std::copy(output.begin(), output.end(), this->value.begin());
    };
    void convert_units(const Quantity::PointerType& from_quantity, const std::string& to_units) override {
      // TODO: use the same BaseValue pointers in the puq to allow variable precision
      std::vector<double> input(this->value.begin(), this->value.end());
      puq::Quantity quantity = input * (*from_quantity);
      quantity = quantity.convert(to_units);
      std::vector<double> output = quantity.value.magnitude.value.value;
      std::copy(output.begin(), output.end(), this->value.begin());
    };
  };
  
  template <>
  class ArrayValue<std::string>: public BaseArrayValue<std::string> {
  public:
    ArrayValue(const std::string& val, const Array::ShapeType& sh, const ValueDtype dt): BaseArrayValue<std::string>(val,sh,dt) {};
    ArrayValue(const Array::StringType&  arr, const Array::ShapeType& sh, const ValueDtype dt): BaseArrayValue<std::string>(arr,sh,dt) {};
    ArrayValue(const std::string& val, const Array::ShapeType& sh): ArrayValue(val,sh,ValueDtype::String) {};
    ArrayValue(const Array::StringType&  arr, const Array::ShapeType& sh): ArrayValue(arr,sh,ValueDtype::String) {};
  private:
    void value_to_string(std::ostringstream& oss, size_t& offset, int precision=0) const override {
      oss << "'" << value[offset] << "'";
    }
  public:
    std::string to_string(int precision=0) const override {
      size_t offset = 0;
      if (precision==0) {
	return to_string_dim(offset);
      } else {
	throw std::runtime_error("String value does not support precision parameter for to_string() method.");
      }
    };
    BaseValue::PointerType clone() const override {
      return std::make_unique<ArrayValue<std::string>>(this->value, this->shape, this->dtype);
    };
    BaseValue::PointerType cast_as(ValueDtype dt) const override;
    BaseValue::PointerType slice(const Array::RangeType& slice) override {
      return this->slice_value(slice);
    };
  };

  template <>
  class ArrayValue<bool>: public BaseArrayValue<bool> {
  public:
    ArrayValue(const bool& val, const Array::ShapeType& sh, const ValueDtype dt): BaseArrayValue<bool>(val,sh,dt) {};
    ArrayValue(const std::vector<bool>&  arr, const Array::ShapeType& sh, const ValueDtype dt): BaseArrayValue<bool>(arr,sh,dt) {};
    ArrayValue(const bool& val, const Array::ShapeType& sh): ArrayValue(val,sh,ValueDtype::Boolean) {};
    ArrayValue(const std::vector<bool>&  arr, const Array::ShapeType& sh): ArrayValue(arr,sh,ValueDtype::Boolean) {};
  private:
    void value_to_string(std::ostringstream& oss, size_t& offset, int precision=0) const override {
      if (value[offset])
	oss << KEYWORD_TRUE;
      else
	oss << KEYWORD_FALSE;
    }
  public:
    std::string to_string(int precision=0) const override {
      size_t offset = 0;
      if (precision==0) {
	return to_string_dim(offset);
      } else {
	throw std::runtime_error("Boolean value does not support precision parameter for to_string() method.");
      }
    };
    BaseValue::PointerType clone() const override {
      return std::make_unique<ArrayValue<bool>>(this->value, this->shape, this->dtype);
    };
    BaseValue::PointerType cast_as(ValueDtype dt) const override;
    BaseValue::PointerType slice(const Array::RangeType& slice) override {
      return this->slice_value(slice);
    };
  };

}

#endif // VAL_VALUES_ARRAY_H
