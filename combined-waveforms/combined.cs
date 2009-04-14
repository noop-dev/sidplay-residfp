using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;

namespace sidwaveforms {
    struct Parameters {
        public float bias, pulsestrength, topbit, distance, stmix;

        public int Score(int wave, int[] reference, bool print) {
            int score = 0;
            for (int j = 0; j < 4096; j ++) {

                /* S */
                var bitarray = new float[12];
                for (int i = 0; i < 12; i ++)
                    bitarray[i] = (j & (1 << i)) != 0 ? 1.0f : 0.0f;
                bitarray[11] *= topbit;

                /* T */
                if ((wave & 3) == 1) {
                    bool top = (j & 2048) != 0;
                    for (int i = 11; i > 0; i --) {
                        if (top) {
                            bitarray[i] = 1.0f - bitarray[i-1];
                        } else {
                            bitarray[i] = bitarray[i-1];
                        }
                    }
                    bitarray[0] = 0;
                }

                /* ST */
                if ((wave & 3) == 3) {
                    for (int i = 11; i > 0; i --) {
                        bitarray[i] = bitarray[i-1] * (1f - stmix) + bitarray[i] * stmix;
                    }
                    bitarray[0] *= stmix;
                }

                SimulateMix(bitarray, wave > 4);

                var simval = GetScore8(bitarray);
                var refval = reference[j];
                score += ScoreResult(simval, refval);

                if (print) {
                    Console.WriteLine("{0} {1} {2}", j, refval, simval);
                }
            }
            return score;
        }

        private void SimulateMix(float[] bitarray, bool HasPulse) {
            var tmp = new float[12];
            var wa = new float[12 + 12 + 1];
            for (int i = 0; i <= 12; i ++) {
                wa[12-i] = wa[12+i] = 1.0f / (1.0f + i * i * distance);
            }

            for (int sb = 0; sb < 12; sb ++) {
                float n = 0;
                float avg = 0;
                for (int cb = 0; cb < 12; cb ++) {
                    float weight = wa[sb - cb + 12];
                    avg += bitarray[cb] * weight;
                    n += weight;
                }
                if (HasPulse) {
                    float weight = wa[sb - 12 + 12];
                    avg += pulsestrength * weight;
                    n += weight;
                }
                tmp[sb] = (bitarray[sb] + avg / n) * 0.5f;
            }
            for (int i = 0; i < 12; i ++)
                bitarray[i] = tmp[i];
        }

        private int GetScore8(float[] bitarray) {
            var result = 0;
            for (int cb = 0; cb < 8; cb ++) {
                if (bitarray[4+cb] > bias)
                    result |= 1 << cb;
            }
            return result;
        }

        private static int ScoreResult(int a, int b) {
            int v = a ^ b;
            int c;
            for (c = 0; v != 0; c ++)
              v &= v - 1;
            return c;
        }
        
        public override string ToString() {
            return string.Format(CultureInfo.InvariantCulture,
@"bestparams.bias = {0}f;
bestparams.pulsestrength = {1}f;
bestparams.topbit = {2}f;
bestparams.distance = {3}f;
bestparams.stmix = {4}f;",
                bias, pulsestrength, topbit, distance, stmix
            );
        }
    }

    class Optimizer {
        static readonly Random random = new Random();

        private static float GetRandomValue() {
            var t = 1f - (float) random.NextDouble() * 0.5f;
            if (random.NextDouble() > 0.5) {
                return 1f / t;
            }
            return t;
        }

        private static void Optimize(int[] reference, int wave, char chip)
        {
            var bestparams = new Parameters();
            if (chip == 'D') {
                switch (wave) {
                    case 3:
// current score 280
bestparams.bias = 0.9347278f;
bestparams.pulsestrength = 0f;
bestparams.topbit = 0f;
bestparams.distance = 1.017948f;
bestparams.stmix = 0.5655234f;
                    break;
                    case 5:
// current score 600
bestparams.bias = 0.8931507f;
bestparams.pulsestrength = 2.483499f;
bestparams.topbit = 0f;
bestparams.distance = 0.03339716f;
bestparams.stmix = 0f;
                    break;
                    case 6:
// current score 613
bestparams.bias = 0.8869214f;
bestparams.pulsestrength = 2.440879f;
bestparams.topbit = 1.680824f;
bestparams.distance = 0.02267573f;
bestparams.stmix = 0f;
                    break;
                    case 7:
// current score 44
bestparams.bias = 0.9266459f;
bestparams.pulsestrength = 0.7393153f;
bestparams.topbit = 0f;
bestparams.distance = 0.0598464f;
bestparams.stmix = 0.1851717f;
                    break;
                }
            }
            if (chip == 'E') {
                switch (wave) {
                    case 3:
// current score 144
bestparams.bias = 0.9689716f;
bestparams.pulsestrength = 0f;
bestparams.topbit = 0f;
bestparams.distance = 1.92f;
bestparams.stmix = 0.718864f;
                    break;
                    case 5:
// current score 166
bestparams.bias = 0.9161022f;
bestparams.pulsestrength = 1.879311f;
bestparams.topbit = 0f;
bestparams.distance = 0.02331964f;
bestparams.stmix = 0f;
                    break;
                    case 6:
// current score 10
bestparams.bias = 0.879145f;
bestparams.pulsestrength = 1.30156f;
bestparams.topbit = 0f;
bestparams.distance = 0.006426161f;
bestparams.stmix = 0f;
                    break;
                    case 7:
// current score 2
bestparams.bias = 0.9493611f;
bestparams.pulsestrength = 0.6681492f;
bestparams.topbit = 0f;
bestparams.distance = 0.04524437f;
bestparams.stmix = 0.1509331f;
                    break;
                }
            }
            if (chip == 'G') {
                switch (wave) {
                    case 3:
// current score 252
bestparams.bias = 0.9393118f;
bestparams.pulsestrength = 0f;
bestparams.topbit = 0f;
bestparams.distance = 1.038816f;
bestparams.stmix = 0.5292149f;
                    break;
                    case 5:
// current score 360
bestparams.bias = 0.8924618f;
bestparams.pulsestrength = 2.01122f;
bestparams.topbit = 0f;
bestparams.distance = 0.03133072f;
bestparams.stmix = 0f;
                    break;
                    case 6:
// current score 668
bestparams.bias = 0.8952018f;
bestparams.pulsestrength = 2.213601f;
bestparams.topbit = 1.705941f;
bestparams.distance = 0.01260567f;
bestparams.stmix = 0f;
                    break;
                    case 7:
// current score 14
bestparams.bias = 0.9322616f;
bestparams.pulsestrength = 0.6173857f;
bestparams.topbit = 0f;
bestparams.distance = 0.06722359f;
bestparams.stmix = 0.2427691f;
                    break;
                }
            }
            if (chip == 'V') {
                switch (wave) {
                    case 3:
// current score 314
bestparams.bias = 0.9738218f;
bestparams.pulsestrength = 0f;
bestparams.topbit = 0.992848f;
bestparams.distance = 2.547508f;
bestparams.stmix = 0.9599405f;
                    break;
                    case 5:
// current score 628
bestparams.bias = 0.9236207f;
bestparams.pulsestrength = 2.19129f;
bestparams.topbit = 0f;
bestparams.distance = 0.1108298f;
bestparams.stmix = 0f;
                    break;
                    case 6:
// current score 593
bestparams.bias = 0.9248214f;
bestparams.pulsestrength = 2.232846f;
bestparams.topbit = 0.9491023f;
bestparams.distance = 0.1313893f;
bestparams.stmix = 0f;
                    break;
                    case 7:
// current score 168
bestparams.bias = 0.9845552f;
bestparams.pulsestrength = 1.380867f;
bestparams.topbit = 0.9621406f;
bestparams.distance = 1.592066f;
bestparams.stmix = 0.9472086f;
                    break;
                }
            }
            if (chip == 'W') {
                switch (wave) {
                    case 3:
// current score 319
bestparams.bias = 0.9686383f;
bestparams.pulsestrength = 0f;
bestparams.topbit = 0.9963791f;
bestparams.distance = 2.21016f;
bestparams.stmix = 0.8968357f;
                    break;
                    case 5:
// current score 784
bestparams.bias = 0.9069195f;
bestparams.pulsestrength = 2.203437f;
bestparams.topbit = 0f;
bestparams.distance = 0.129717f;
bestparams.stmix = 0f;
                    break;
                    case 6:
// current score 764
bestparams.bias = 0.9074827f;
bestparams.pulsestrength = 2.184185f;
bestparams.topbit = 0.9717882f;
bestparams.distance = 0.1274436f;
bestparams.stmix = 0f;
                    break;
                    case 7:
// current score 213
bestparams.bias = 0.9882526f;
bestparams.pulsestrength = 1.729343f;
bestparams.topbit = 0.9398671f;
bestparams.distance = 2.652093f;
bestparams.stmix = 0.9998723f;
                    break;
                }
            }

            var bestscore = bestparams.Score(wave, reference, true);
            Console.Write("// initial score {0}\n\n", bestscore);
            while (true) {
                var p = bestparams;
                p.bias *= GetRandomValue();
                p.pulsestrength *= GetRandomValue();
                p.topbit *= GetRandomValue();
		p.distance *= GetRandomValue();
		p.stmix *= GetRandomValue();
                if (p.stmix > 1f)
                    p.stmix = 1f;

                var score = p.Score(wave, reference, false);
                /* accept if improvement */
                if (score <= bestscore) {
                    bestparams = p;
                    bestscore = score;
                    Console.Write("// current score {0}\n{1}\n\n", score, bestparams);
                }
            }
        }

        private static int[] ReadChip(int wave, char chip) {
            Console.WriteLine("Reading chip: {0}", chip);
            var result = new int[4096];
            int i = 0;
            foreach (var line in
                File.ReadAllLines(string.Format("sidwaves/WAVE{0:X}.CSV", wave))
            ) {
                var values = line.Split(',');
                result[i ++] = Convert.ToInt32(values[chip - 'A']);
            }
            return result;
        }
        
        public static void Main(string[] args) {
            var wave = int.Parse(args[0]);
            Debug.Assert(wave == 3 || wave == 5 || wave == 6 || wave == 7);

            var chip = char.Parse(args[1]);
            Debug.Assert(chip >= 'A' && chip <= 'Z');

            var reference = ReadChip(wave, chip);
            Optimize(reference, wave, chip);
        }
    }
}
