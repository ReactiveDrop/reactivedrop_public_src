// Better to use built-in MT19937 RNG provided by c++ STL for highest performance.
// void RandomHQSetSeed(int seed) // Do not call this unless necessary. RNG is automatically seeded upon game start.
// void RandomHQSetRandomDevice() // Do not call this unless necessary. RNG is automatically seeded upon game start.
// int RandomHQUniformIntDistribution(int min, int max)
// float RandomHQUniformFloatDistribution(float min, float max)
// float RandomHQNormalDistribution(float mean, float std_dev)


// 基于 Mersenne Twister ( MT19937 )的高质量随机数生成器，具有无偏的均匀分布和正态分布。
_mtArray <- array(624); // 状态数组
_mtIndex <- 0;

function RngSetSeed(seed) {
	_mtArray[0] = seed;
	for (local i = 1; i < 624; i++) {
		_mtArray[i] = (0x6c078965 * (_mtArray[i - 1] ^ (_mtArray[i - 1] >>> 30)) + i) & 0xffffffff;
	}
	_mtIndex = 624;
}

// 提取随机数
function _RngExtractNumber() {
	// 触发条件：索引越界时扭转
	if (_mtIndex >= 624) {
		_RngTwist();
	}

	local y = _mtArray[_mtIndex++];
	y = y ^ (y >>> 11);
	y = y ^ ((y << 7) & 0x9d2c5680);
	y = y ^ ((y << 15) & 0xefc60000);
	y = y ^ (y >>> 18);

	return y & 0xffffffff;
}

// 打乱状态数组，生成新的随机数
function _RngTwist() {
	for (local i = 0; i < 624; i++) {
		local y = (_mtArray[i] & 0x80000000) + (_mtArray[(i + 1) % 624] & 0x7fffffff);
		_mtArray[i] = _mtArray[(i + 397) % 624] ^ (y >>> 1);

		if (y % 2 != 0) {
			_mtArray[i] = _mtArray[i] ^ 0x9908b0df;
		}
	}
	_mtIndex = 0;
}

// 丢掉符号位，得到[ 0, 0x7fffffff ]的随机整数
function _RngRandom() {
	return _RngExtractNumber() & 0x7fffffff;
}

// 得到[ min, max ]之间均匀分布的随机整数
function RandIntUniformDistribution(min, max) {
	local range = max - min + 1;
	local num;

	// 设置随机数范围上限
	local securemax = (0x7fffffff - (0x7fffffff % range));
	do {
		num = _RngRandom();
	} while (num >= securemax); // 拒绝使用可能导致概率偏差的数

	return min + (num % range); // 返回 [ min, max ]之间的均匀分布
}

// 获得 [ min, max ]之间的均匀小数分布
// 实际上，由于float的有效数字问题，两个相邻的随机整数经过计算后可能等于同一个浮点数
// 因此这个方法并不能生成绝对均匀分布的小数。特别是对于区间端点而言
function RandFloatUniformDistribution(min, max) {
	// 生成 [ 0, 1 ]的均匀分布的随机小数
	local randomFloat = _RngRandom().tofloat() / 0x7fffffff;
	return min + (randomFloat * (max - min)); // 将区间缩放到目标区间
}
// 生成符合正态分布（高斯分布）的随机数
// mean: 正态分布的均值
// std_dev: 正态分布的标准差
function RandFloatNormalDistribution(mean, std_dev) {
	local u1 = _RngRandom().tofloat() / 0x7fffffff; // Uniform( 0, 1 )
	local u2 = _RngRandom().tofloat() / 0x7fffffff; // Uniform( 0, 1 )

	// Box-Muller 变换，得到标准正态分布
	local z0 = sqrt(-2 * log(u1)) * cos(2 * 3.14159 * u2);
	// Transform Z0 to have the desired mean and standard deviation
	return mean + z0 * std_dev; // 变换到目标分布
}

// Fisher-Yates 洗牌算法，理论上保证均匀均匀洗牌
function FisherYatesShuffle(arr) {
	// 遍历数组，从后往前随机交换
	for (local i = arr.len() - 1; i > 0; i--) {
		// 生成一个随机索引
		local j = RandomHQUniformIntDistribution(0, i);

		// 交换元素
		local tmp = arr[i];
		arr[i] = arr[j];
		arr[j] = tmp;
	}
	return arr;
}