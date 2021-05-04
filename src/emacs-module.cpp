#include "emacs-module.hpp"

#include <forward_list>

namespace emacs
{
  ptrdiff_t runtime::size() const EMACS_NOEXCEPT { return m_runtime.size; }
  env& runtime::get_environment() EMACS_NOEXCEPT 
  {
	auto ptr = reinterpret_cast<env*>(m_runtime.get_environment(&m_runtime));
	return *ptr;
  }

  value env_25::function_caller(emacs_env* pEnv,
								ptrdiff_t nargs,
								value* args,
								void* data) EMACS_NOEXCEPT
  {
	auto func = reinterpret_cast<value(*)(emacs::env&, ptrdiff_t, value*)>(data);
	auto& env = *reinterpret_cast<emacs::env*>(pEnv);
	return func(env, nargs, args);
  }

  value env_25::callable_caller(emacs_env* pEnv,
								ptrdiff_t nargs,
								value* args,
								void* pCallable) EMACS_NOEXCEPT
  {
	auto& callable = *reinterpret_cast<callable_t*>(pCallable);
	auto& env = *reinterpret_cast<emacs::env*>(pEnv);
	return callable(env, nargs, args);
  }

  static std::forward_list<std::function<value(env&, ptrdiff_t, value*)>> callables;

  env_25::callable_t* env_25::push_callable(callable_t&& functor)
  {
	callables.push_front(std::move(functor));
	return &callables.front();
  }

  value env_25::make_global_ref(value any_reference) EMACS_NOEXCEPT
  {
	return m_env.make_global_ref(&m_env, any_reference);
  }

  void env_25::free_global_ref(value global_reference) EMACS_NOEXCEPT
  {
	m_env.free_global_ref(&m_env, global_reference);
  }

  funcall_exit env_25::non_local_exit_check() EMACS_NOEXCEPT
  {
	return static_cast<funcall_exit>(m_env.non_local_exit_check(&m_env));
  }

  void env_25::non_local_exit_clear() EMACS_NOEXCEPT
  {
	m_env.non_local_exit_clear(&m_env);
  }

  funcall_exit env_25::non_local_exit_get(value* non_local_exit_symbol_out,
										  value *non_local_exit_data_out) EMACS_NOEXCEPT
  {
	return static_cast<funcall_exit>(m_env.non_local_exit_get(&m_env,
															  non_local_exit_symbol_out,
															  non_local_exit_data_out));
  }

  void env_25::non_local_exit_signal(value non_local_exit_symbol,
									 value non_local_exit_data) EMACS_NOEXCEPT
  {
	m_env.non_local_exit_signal(&m_env, non_local_exit_symbol, non_local_exit_data);
  }

  void env_25::non_local_exit_throw(value tag,
									value val) EMACS_NOEXCEPT
  {
	m_env.non_local_exit_throw(&m_env, tag, val);
  }

  value env_25::make_function(ptrdiff_t min_arity,
							  ptrdiff_t max_arity,
							  value (*function) (emacs_env *env,
												 ptrdiff_t nargs,
												 value args[],
												 void *) EMACS_NOEXCEPT,
							  const char *documentation,
							  void *data) EMACS_NOEXCEPT
  {
	return m_env.make_function(&m_env, 
							   min_arity,
							   max_arity,
							   function,
							   documentation,
							   data);	
  }

  value env_25::funcall(value function,
						ptrdiff_t nargs,
						value args[]) EMACS_NOEXCEPT
  {
	return m_env.funcall(&m_env, function, nargs, args);
  }

  value env_25::funcall(value function,
						std::span<value> args) EMACS_NOEXCEPT
  {
	return funcall(function, args.size(), args.data());
  }

  value env_25::funcall(value function, value arg) EMACS_NOEXCEPT
  {
	return funcall(function, 1, &arg);
  }

  value env_25::intern(const char* symbol_name) EMACS_NOEXCEPT
  {
	return m_env.intern(&m_env, symbol_name);
  }

  value env_25::intern_and_eval(const char* symbol_name) EMACS_NOEXCEPT
  {
	return funcall(intern("eval"), intern(symbol_name));
  }

  value env_25::type_of(value val) EMACS_NOEXCEPT
  {
	return m_env.type_of(&m_env, val);	
  }

  bool env_25::is_not_nil(value val) EMACS_NOEXCEPT
  {
	return m_env.is_not_nil(&m_env, val);
  }

  bool env_25::eq(value a, value b) EMACS_NOEXCEPT
  {
	return m_env.eq(&m_env, a, b);
  }

  intmax_t env_25::extract_integer(value val) EMACS_NOEXCEPT
  {
	return m_env.extract_integer(&m_env, val);	
  }

  value env_25::make_integer(intmax_t val) EMACS_NOEXCEPT
  {
	return m_env.make_integer(&m_env, val);	
  }

  double env_25::extract_float(value val) EMACS_NOEXCEPT
  {
	return m_env.extract_float(&m_env, val);	
  }

  value env_25::make_float(double val) EMACS_NOEXCEPT
  {
	return m_env.make_float(&m_env, val);	
  }

  bool env_25::copy_string_contents(value val,
									char* buffer,
									ptrdiff_t* size_inout) EMACS_NOEXCEPT
  {
	return m_env.copy_string_contents(&m_env, val, buffer, size_inout);
  }

  value env_25::make_string(const char* contents, ptrdiff_t length) EMACS_NOEXCEPT
  {
	return m_env.make_string(&m_env, contents, length);	
  }

  value env_25::make_string(std::string_view string) EMACS_NOEXCEPT
  {
	return make_string(string.data(), string.size());
  }

  value env_25::make_user_ptr(void(*fin)(void*) EMACS_NOEXCEPT,
							  void* ptr) EMACS_NOEXCEPT
  {
	return m_env.make_user_ptr(&m_env, fin, ptr);
  }

  void* env_25::get_user_ptr(value uptr) EMACS_NOEXCEPT
  {
	return m_env.get_user_ptr(&m_env, uptr);	
  }

  void env_25::set_usert_ptr(value uptr, void* ptr) EMACS_NOEXCEPT
  {
	m_env.set_user_ptr(&m_env, uptr, ptr);
  }

  void (*env_25::get_user_finalizer (value uptr))(void *) EMACS_NOEXCEPT
  {
	return m_env.get_user_finalizer(&m_env, uptr);
  }

  void env_25::set_user_finalizer(value uptr,
								  void (*fin) (void *) EMACS_NOEXCEPT) EMACS_NOEXCEPT
  {
	m_env.set_user_finalizer(&m_env, uptr, fin);
  }

  value env_25::vec_get(value vec, ptrdiff_t i) EMACS_NOEXCEPT
  {
	return m_env.vec_get(&m_env, vec, i);
  }

  void env_25::vec_set(value vec, ptrdiff_t i, value val) EMACS_NOEXCEPT
  {
	m_env.vec_set(&m_env, vec, i, val);
  }

  ptrdiff_t env_25::vec_size(value vec) EMACS_NOEXCEPT
  {
	return m_env.vec_size(&m_env, vec);
  }

  bool env_26::should_quit() EMACS_NOEXCEPT
  {
	return m_env.should_quit(&m_env);
  }

  process_input_result env_27::process_input() EMACS_NOEXCEPT
  {
	return static_cast<process_input_result>(m_env.process_input(&m_env));
  }

  timespec env_27::extract_time(value val) EMACS_NOEXCEPT
  {
	return m_env.extract_time(&m_env, val);
  }

  value env_27::make_time(timespec time) EMACS_NOEXCEPT
  {
	return m_env.make_time(&m_env, time);
  }

  bool env_27::extract_big_integer(value arg, int* sign,
								   ptrdiff_t* count, limb_t* magnitude) EMACS_NOEXCEPT
  {
	return m_env.extract_big_integer(&m_env, arg, sign, count, magnitude);
  }

  value env_27::make_big_integer(int sign, ptrdiff_t count, const limb_t* magnitude) EMACS_NOEXCEPT
  {
	return m_env.make_big_integer(&m_env, sign, count, magnitude);
  }
}

extern "C" int 
emacs_module_init(struct emacs_runtime* ert)
{
  return emacs::module_init(*reinterpret_cast<emacs::runtime*>(ert));
}
