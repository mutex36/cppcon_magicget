namespace Core::Reflection
{
	namespace _Detail
	{
		template<typename T>
		struct type_to_hash;

		template<>
		struct type_to_hash<int> {
			static constexpr size_t hash = 1;
		};

		template<>
		struct type_to_hash<float> {
			static constexpr size_t hash = 2;
		};

		template<>
		struct type_to_hash<double> {
			static constexpr size_t hash = 3;
		};

		template<size_t I>
		struct hash_to_type;

		template<>
		struct hash_to_type<1> {
			typedef int type;
		};

		template<>
		struct hash_to_type<2> {
			typedef float type;
		};

		template<>
		struct hash_to_type<3> {
			typedef double type;
		};

		template<size_t I>
		struct ubiq {
			template<typename T>
			constexpr operator T() const noexcept {
				return T{};
			}
		};

		template<size_t I>
		struct ubiq_val {
			size_t& _hash;

			template<typename T>
			constexpr operator T() const noexcept {
				_hash = type_to_hash<std::decay_t<T>>::hash;
				return T{};
			}
		};

		template<typename T, size_t I0, size_t...I>
		constexpr auto count_fields_impl(size_t& out, std::index_sequence<I0, I...>)
			-> decltype(T{ ubiq<I0>{}, ubiq<I>{}... }) {
			out = sizeof...(I)+1;
			return {};
		}

		template<typename T, size_t...I>
		constexpr void count_fields_impl(size_t& out, std::index_sequence<I...>) {
			count_fields_impl<T>(out, std::make_index_sequence<sizeof...(I)-1>{});
		}

		template<typename T>
		constexpr size_t count_fields() {
			size_t out = 0;
			count_fields_impl<T>(out, std::make_index_sequence<20>());
			return out;
		}

		template<typename T, size_t...N, size_t N0>
		constexpr auto get_field_hash_impl(size_t& out, std::index_sequence<N0, N...>) {
			T temp{ ubiq<N>{}..., ubiq_val<N0>{out} };
			return temp;
		}

		template<typename T, size_t Index>
		constexpr size_t get_field_hash() {
			size_t out = 0;
			get_field_hash_impl<T>(out, std::make_index_sequence<Index + 1>{});
			return out;
		}

		template<typename T, size_t Index>
		struct field_type {
			using type = typename _Detail::hash_to_type<
				_Detail::get_field_hash<T, Index>()>::type;
		};
	}

	template<typename T>
	struct Reflect {
		static_assert(std::is_class<T>::value, "reflected type is not a struct/class");

		static constexpr size_t field_count = _Detail::count_fields<T>();

		template<size_t Index>
		using field_type = typename _Detail::field_type<T, Index>::type;
	};
}
