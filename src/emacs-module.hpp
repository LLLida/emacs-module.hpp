#pragma once

#include <emacs-module.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string_view>
#include <type_traits>

namespace emacs
{
// Some versions of emacs-module.h doesn't
// define EMACS_MAJOR_VERSION(termux).
// The only choice we have is to use the oldest version - 25
#ifndef EMACS_MAJOR_VERSION
#  define EMACS_MAJOR_VERSION 25
#endif

  class env_25;
#if EMACS_MAJOR_VERSION >= 26
  class env_26;
#endif
#if EMACS_MAJOR_VERSION >= 27
  class env_27;
#endif
#if EMACS_MAJOR_VERSION >= 28
  class env_28;
#endif

#if EMACS_MAJOR_VERSION == 25
  using env = env_25;
#elif EMACS_MAJOR_VERSION == 26
  using env = env_26;
#elif EMACS_MAJOR_VERSION == 27
  using env = env_27;
#elif EMACS_MAJOR_VERSION == 28
  using env = env_28;
#endif

  class runtime
  {
  private:
	emacs_runtime m_runtime;

  public:
	ptrdiff_t size() const EMACS_NOEXCEPT;
	env& get_environment() EMACS_NOEXCEPT;
  };

  enum class funcall_exit {
	RETURN = 0,
	SIGNAL = 1,
	THROW = 2,
  };

  enum class process_input_result {
	CONTINUE = 0,
	QUIT = 1,
  };

// Unfortunately some versions of emacs-module.h doesn't
// define emacs_limb_t(termux).
// We hope unsigned int will do the thing.
#ifdef EMACS_LIMB_MAX
  using limb_t = emacs_limb_t;
#else
  using limb_t = unsigned int;
#endif

  using value = emacs_value;

  class env_25
  {
  protected:
	emacs_env m_env;

	static value function_caller(emacs_env* pEnv,
								 ptrdiff_t nargs,
								 value* args,
								 void* pFunction) EMACS_NOEXCEPT;
	static value callable_caller(emacs_env* pEnv,
								 ptrdiff_t nargs,
								 value* args,
								 void* pCallable) EMACS_NOEXCEPT;
	using callable_t = std::function<value(env&, ptrdiff_t, value*)>;
	callable_t* push_callable(callable_t&& functor);

  public:
	env_25(const env_25&) = delete;
	env_25(env_25&&) = delete;
	env_25& operator=(const env_25&) = delete;
	env_25& operator=(env_25&&) = delete;

	[[nodiscard]]
	value make_global_ref(value any_reference) EMACS_NOEXCEPT; 	
	void free_global_ref(value global_reference) EMACS_NOEXCEPT;
	[[nodiscard]]
	funcall_exit non_local_exit_check() EMACS_NOEXCEPT;
	void non_local_exit_clear() EMACS_NOEXCEPT;
	[[nodiscard]]
	funcall_exit non_local_exit_get(value* non_local_exit_symbol_out,
									value *non_local_exit_data_out) EMACS_NOEXCEPT;
	void non_local_exit_signal(value non_local_exit_symbol,
							   value non_local_exit_data) EMACS_NOEXCEPT;
	void non_local_exit_throw(value tag,
							  value val) EMACS_NOEXCEPT;
	/**
	 * @brief Make lisp function from C function.
	 * @detail This overload is the fastest way to make lisp function, it
	 * directly calls the C API.
	 * @param min_arity  specifies the minimum number of arguments that `function` can accept
	 * @param max_arity  specifies the maximum number of arguments that `function` can accept
	 * @param function - module function. The `env` argument provides a pointer to the API environment, 
	 * needed to access Emacs objects and functions. The `nargs` argument 
	 * is the required number of arguments, which can be zero, and `args`
	 * is a pointer to the array of the function arguments.
	 * The argument `data` points to additional data required by the function,
	 * which was arranged when make_function was called to create an Emacs function from module_func.
	 * @param documentation function's docstring
	 * @param data pointer that will be passed to the function everytime it is called
	 */
	[[nodiscard]]
	value make_function(ptrdiff_t min_arity,
						ptrdiff_t max_arity,
						value (*function) (emacs_env *env,
										   ptrdiff_t nargs,
										   value args[],
										   void* data) EMACS_NOEXCEPT,
						const char *documentation,
						void *data) EMACS_NOEXCEPT;
	/**
	 * @brief Make lisp function from C++ function.
	 * @detail This overload is almost fast as one that takes C function.
	 * It is recommened to use this overload. `function` can be captureless lambda.
	 * @param min_arity  specifies the minimum number of arguments that `function` can accept
	 * @param max_arity  specifies the maximum number of arguments that `function` can accept
	 * @param function - module function with signature:
	 * emacs::value(emacs::env& env, ptrdiff_t nargs, emacs::value* args).
	 * The `env` argument provides a pointer to the API environment, 
	 * needed to access Emacs objects and functions. The `nargs` argument 
	 * is the required number of arguments, which can be zero, and `args`
	 * is a pointer to the array of the function arguments.
	 * The argument `data` points to additional data required by the function,
	 * which was arranged when make_function was called to create an Emacs function from module_func.
	 * @param documentation function's docstring
	 */
	template<typename F> [[nodiscard]]
	value make_function(ptrdiff_t min_arity,
						ptrdiff_t max_arity,
						F&& function,
						const char* documentation) EMACS_NOEXCEPT
	  requires std::is_convertible_v<F, value(*)(env&, ptrdiff_t, value*)>
	{
	  auto rawFunc = static_cast<value(*)(env&, ptrdiff_t, value*)>(function);
	  return make_function(min_arity,
						   max_arity,
						   function_caller,
						   documentation,
						   reinterpret_cast<void*>(rawFunc));
	}
	/**
	 * @brief Make lisp function from C++ callable object.
	 * @detail This overload can be slow, it does 1 memory allocation
	 * to store callable object. `function` can be capturefull lambda.
	 * @param min_arity  specifies the minimum number of arguments that `function` can accept
	 * @param max_arity  specifies the maximum number of arguments that `function` can accept
	 * @param function - module function with signature:
	 * emacs::value(emacs::env& env, ptrdiff_t nargs, emacs::value* args).
	 * The `env` argument provides a pointer to the API environment, 
	 * needed to access Emacs objects and functions. The `nargs` argument 
	 * is the required number of arguments, which can be zero, and `args`
	 * is a pointer to the array of the function arguments.
	 * The argument `data` points to additional data required by the function,
	 * which was arranged when make_function was called to create an Emacs function from module_func.
	 * @param documentation function's docstring
	 */
	template<typename F> [[nodiscard]]
	value make_function(ptrdiff_t min_arity,
						ptrdiff_t max_arity,
						F&& function,
						const char* documentation)
	{
	  auto pCallable = push_callable(function);
	  return make_function(min_arity,
						   max_arity,
						   callable_caller,
						   documentation,
						   pCallable);
	}
	value funcall(value function,
				  ptrdiff_t nargs,
				  value args[]) EMACS_NOEXCEPT;
	value funcall(value function,
				  std::span<value> args) EMACS_NOEXCEPT;
	value funcall(value function, value arg) EMACS_NOEXCEPT;
	[[nodiscard]]
	value intern(const char* symbol_name) EMACS_NOEXCEPT;
	[[nodiscard]]
	value intern_and_eval(const char* symbol_name) EMACS_NOEXCEPT;
	[[nodiscard]]
	value type_of(value val) EMACS_NOEXCEPT;
	[[nodiscard]]
	bool is_not_nil(value val) EMACS_NOEXCEPT;
	[[nodiscard]]
	bool eq(value a, value b) EMACS_NOEXCEPT;
	[[nodiscard]]
	intmax_t extract_integer(value val) EMACS_NOEXCEPT;
	[[nodiscard]]
	value make_integer(intmax_t val) EMACS_NOEXCEPT;
	[[nodiscard]]
	double extract_float(value val) EMACS_NOEXCEPT;
	[[nodiscard]]
	value make_float(double val) EMACS_NOEXCEPT;
	[[nodiscard]]
	bool copy_string_contents(value val,
								char* buffer,
								ptrdiff_t* size_inout) EMACS_NOEXCEPT;
	[[nodiscard]]
	value make_string(const char* contents, ptrdiff_t length) EMACS_NOEXCEPT;
	[[nodiscard]]
	value make_string(std::string_view string) EMACS_NOEXCEPT;
	[[nodiscard]]
	value make_user_ptr(void(*fin)(void*) EMACS_NOEXCEPT,
						void* ptr) EMACS_NOEXCEPT;
	[[nodiscard]]
	void* get_user_ptr(value uptr) EMACS_NOEXCEPT;
	void set_usert_ptr(value uptr, void* ptr) EMACS_NOEXCEPT;
	void (*get_user_finalizer (value uptr))(void *) EMACS_NOEXCEPT;
	void set_user_finalizer(value uptr,
							void (*fin) (void *) EMACS_NOEXCEPT) EMACS_NOEXCEPT;
	[[nodiscard]]
	value vec_get(value vec, ptrdiff_t i) EMACS_NOEXCEPT;
	void vec_set(value vec, ptrdiff_t i, value val) EMACS_NOEXCEPT;
	[[nodiscard]]
	ptrdiff_t vec_size(value vec) EMACS_NOEXCEPT;
  };

#if EMACS_MAJOR_VERSION >= 26
  class env_26 : public env_25
  {
  public:
	[[nodiscard]]
	bool should_quit() EMACS_NOEXCEPT;
  };
#endif

#if EMACS_MAJOR_VERSION >= 27
  class env_27 : public env_26
  {
  public:
	[[nodiscard]]
	process_input_result process_input() EMACS_NOEXCEPT;
	[[nodiscard]]
	timespec extract_time(value val) EMACS_NOEXCEPT;
	[[nodiscard]]
	value make_time(timespec time) EMACS_NOEXCEPT;
	[[nodiscard]]
	bool extract_big_integer(value arg, int* sign,
							 ptrdiff_t* count, limb_t* magnitude) EMACS_NOEXCEPT;
	[[nodiscard]]
	value make_big_integer(int sign, ptrdiff_t count,
						   const limb_t* magnitude) EMACS_NOEXCEPT;
  };
#endif

#if EMACS_MAJOR_VERSION >= 28
  class env_28 : public env_27
  {
  public:
	void (*get_function_finalizer (value arg))(void *) EMACS_NOEXCEPT;
	void set_function_finalizer(value arg,
								void (*fin) (void *) EMACS_NOEXCEPT) EMACS_NOEXCEPT;
	[[nodiscard]]
	int open_channel(value pipe_process) EMACS_NOEXCEPT;
	void make_interactive(value function, value spec) EMACS_NOEXCEPT;
	[[nodiscard]]
	value make_unibyte_string(const char* str, ptrdiff_t len) EMACS_NOEXCEPT;
  };
#endif
  
  /**
   * @brief The entry function of the module. You should define it.
   */
  int module_init(runtime& ert) EMACS_NOEXCEPT;
}
