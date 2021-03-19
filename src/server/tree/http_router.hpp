#pragma once
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <string_view>

#include "tree.h"
#include "function_traits.hpp"

using radixtree::HandleFunc;
using radixtree::ParseResult;
using radixtree::RadixTree;
using radixtree::request;
using radixtree::response;
namespace cinatra
{
	//for is_detective

	struct nonesuch
	{
		nonesuch() = delete;
		~nonesuch() = delete;
		nonesuch(const nonesuch &) = delete;
		void operator=(const nonesuch &) = delete;
	};

	template <class Default, class AlwaysVoid,
			  template <class...> class Op, class... Args>
	struct detector
	{
		using value_t = std::false_type;
		using type = Default;
	};

	template <class Default, template <class...> class Op, class... Args>
	struct detector<Default, std::void_t<Op<Args...>>, Op, Args...>
	{
		using value_t = std::true_type;
		using type = Op<Args...>;
	};

	template <template <class...> class Op, class... Args>
	using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

	template <template <class...> class Op, class... Args>
	using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

	template <class T, typename... Args>
	using has_before_t = decltype(std::declval<T>().before(std::declval<Args>()...));

	template <class T, typename... Args>
	using has_after_t = decltype(std::declval<T>().after(std::declval<Args>()...));
	template <typename T, typename... Args>
	using has_before = is_detected<has_before_t, T, Args...>;

	template <typename T, typename... Args>
	using has_after = is_detected<has_after_t, T, Args...>;

	template <typename... Args, typename F, std::size_t... Idx>
	constexpr void for_each_l(std::tuple<Args...> &t, F &&f, std::index_sequence<Idx...>)
	{
		(std::forward<F>(f)(std::get<Idx>(t)), ...);
	}

	template <typename... Args, typename F, std::size_t... Idx>
	constexpr void for_each_r(std::tuple<Args...> &t, F &&f, std::index_sequence<Idx...>)
	{
		constexpr auto size = sizeof...(Idx);
		(std::forward<F>(f)(std::get<size - Idx - 1>(t)), ...);
	}

	enum class http_method
	{
		UNKNOW,
		DEL,
		GET,
		HEAD,
		POST,
		PUT,
		CONNECT,
		OPTIONS,
		TRACE
	};
	constexpr inline auto GET = http_method::GET;
	constexpr inline auto POST = http_method::POST;
	constexpr inline auto DEL = http_method::DEL;
	constexpr inline auto HEAD = http_method::HEAD;
	constexpr inline auto PUT = http_method::PUT;
	constexpr inline auto CONNECT = http_method::CONNECT;
	constexpr inline auto TRACE = http_method::TRACE;
	constexpr inline auto OPTIONS = http_method::OPTIONS;
	using namespace std::string_view_literals;
	constexpr auto type_to_name(std::integral_constant<http_method, http_method::DEL>) noexcept { return "DELETE"sv; }
	constexpr auto type_to_name(std::integral_constant<http_method, http_method::GET>) noexcept { return "GET"sv; }
	constexpr auto type_to_name(std::integral_constant<http_method, http_method::HEAD>) noexcept { return "HEAD"sv; }

	constexpr auto type_to_name(std::integral_constant<http_method, http_method::POST>) noexcept { return "POST"sv; }
	constexpr auto type_to_name(std::integral_constant<http_method, http_method::PUT>) noexcept { return "PUT"sv; }

	constexpr auto type_to_name(std::integral_constant<http_method, http_method::CONNECT>) noexcept { return "CONNECT"sv; }
	constexpr auto type_to_name(std::integral_constant<http_method, http_method::OPTIONS>) noexcept { return "OPTIONS"sv; }
	constexpr auto type_to_name(std::integral_constant<http_method, http_method::TRACE>) noexcept { return "TRACE"sv; }
	namespace
	{
		constexpr char DOT = '.';
		constexpr char SLASH = '/';
		constexpr std::string_view INDEX = "index";
	}

	class http_router
	{
	public:
		template <http_method... Is, typename Function, typename... Ap>
		std::enable_if_t<!std::is_member_function_pointer_v<Function>> register_handler(const std::string &name, Function &&f, const Ap &...ap)
		{
			if constexpr (sizeof...(Is) > 0)
			{
				auto arr = get_method_arr<Is...>();
				register_nonmember_func(name, arr, std::forward<Function>(f), ap...);
			}
			else
			{
				register_nonmember_func(name, {}, std::forward<Function>(f), ap...);
			}
		}

		template <http_method... Is, class T, class Type, typename T1, typename... Ap>
		std::enable_if_t<std::is_same_v<T *, T1>> register_handler(std::string_view name, Type (T::*f)(request &, response &), T1 t, const Ap &...ap)
		{
			register_handler_impl<Is...>(name, f, t, ap...);
		}

		template <http_method... Is, class T, class Type, typename... Ap>
		void register_handler(std::string_view name, Type (T::*f)(request &, response &), const Ap &...ap)
		{
			register_handler_impl<Is...>(name, f, (T *)nullptr, ap...);
		}

		void remove_handler(std::string name)
		{
			// not support
			//this->map_invokers_.erase(name);
		}

		//elimate exception, resut type bool: true, success, false, failed
		bool route(const std::string &method, const std::string url, request &req, response &res)
		{

			auto ret = tree.get(url, method);
			if (!std::get<0>(ret))
			{
				// not such handler
				return false;
			}
			// set the parameter vector
			req.setPara(std::move(std::get<2>(ret)));
			// invoke handler

			(std::get<1>(ret))(req, res);
			return true;
		}

	private:
		template <http_method... Is, class T, class Type, typename T1, typename... Ap>
		void register_handler_impl(std::string_view name, Type T::*f, T1 t, const Ap &...ap)
		{
			if constexpr (sizeof...(Is) > 0)
			{
				auto arr = get_method_arr<Is...>();
				register_member_func(name, arr, f, t, ap...);
			}
			else
			{
				register_member_func(name, {0}, f, t, ap...);
			}
		}

		template <typename Function, typename... AP>
		void register_nonmember_func(const std::string raw_name, const std::vector<std::string_view> &arr, Function f, const AP &...ap)
		{
			tree.insert(raw_name, std::bind(&http_router::invoke<Function, AP...>, this, std::placeholders::_1, std::placeholders::_2, std::move(f), ap...), arr);
		}

		template <typename Function, typename... AP>
		void invoke(request &req, response &res, Function f, AP... ap)
		{
			using result_type = std::result_of_t<Function(request &, response &)>;
			std::tuple<AP...> tp(std::move(ap)...);
			bool r = do_ap_before(req, res, tp);

			if (!r)
				return;

			if constexpr (std::is_void_v<result_type>)
			{
				//business
				f(req, res);
				//after
				do_void_after(req, res, tp);
			}
			else
			{
				//business
				result_type result = f(req, res);
				//after
				do_after(std::move(result), req, res, tp);
			}
		}

		template <typename Function, typename Self, typename... AP>
		void register_member_func(std::string_view raw_name, const std::vector<std::string_view> &arr, Function f, Self self, const AP &...ap)
		{

			tree.insert(raw_name, std::bind(&http_router::invoke_mem<Function, Self, AP...>, this, std::placeholders::_1, std::placeholders::_2, f, self, ap...));
		}

		template <typename Function, typename Self, typename... AP>
		void invoke_mem(request &req, response &res, Function f, Self self, AP... ap)
		{
			using result_type = typename timax::function_traits<Function>::result_type;

			std::tuple<AP...> tp(std::move(ap)...);
			bool r = do_ap_before(req, res, tp);

			if (!r)
				return;
			using nonpointer_type = std::remove_pointer_t<Self>;
			if constexpr (std::is_void_v<result_type>)
			{
				//business
				if (self)
					(*self.*f)(req, res);
				else
					(nonpointer_type{}.*f)(req, res);
				//after
				do_void_after(req, res, tp);
			}
			else
			{
				//business
				result_type result;
				if (self)
					result = (*self.*f)(req, res);
				else
					result = (nonpointer_type{}.*f)(req, res);
				//after
				do_after(std::move(result), req, res, tp);
			}
		}

		template <typename Tuple>
		bool do_ap_before(request &req, response &res, Tuple &tp)
		{
			bool r = true;
			for_each_l(
				tp, [&r, &req, &res](auto &item) {
					if (!r)
						return;

					constexpr bool has_befor_mtd = has_before<decltype(item), request &, response &>::value;
					if constexpr (has_befor_mtd)
						r = item.before(req, res);
				},
				std::make_index_sequence<std::tuple_size_v<Tuple>>{});

			return r;
		}

		template <typename Tuple>
		void do_void_after(request &req, response &res, Tuple &tp)
		{
			bool r = true;
			for_each_r(
				tp, [&r, &req, &res](auto &item) {
					if (!r)
						return;

					constexpr bool has_after_mtd = has_after<decltype(item), request &, response &>::value;
					if constexpr (has_after_mtd)
						r = item.after(req, res);
				},
				std::make_index_sequence<std::tuple_size_v<Tuple>>{});
		}

		template <typename T, typename Tuple>
		void do_after(T &&result, request &req, response &res, Tuple &tp)
		{
			bool r = true;
			for_each_r(
				tp, [&r, result = std::move(result), &req, &res](auto &item) {
					if (!r)
						return;

					if constexpr (has_after<decltype(item), T, request &, response &>::value)
						r = item.after(std::move(result), req, res);
				},
				std::make_index_sequence<std::tuple_size_v<Tuple>>{});
		}

		template <http_method... Is>
		constexpr auto get_method_arr()
		{
			std::vector<std::string_view> arr;
			std::string_view s;
			((s = type_to_name(std::integral_constant<http_method, Is>{}), arr.push_back(s)), ...);
			return arr;
		}

		//	typedef std::pair<std::array<char, 26>, std::function<void(request &, response &)>> invoker_function;
		//	std::map<std::string_view, invoker_function> map_invokers_;

		RadixTree tree;
	};
}
