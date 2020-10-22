using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace SquareWaveConverter
{
    class Program
    {
        struct Pulse
        {
            public bool IsHigh;
            public double MS;
        }

        class Code
        {
            public List<Pulse> Pulse = new List<Pulse>();
        }

        static void WriteHeader(BinaryReader rdr)
        {
            var chunkId = rdr.ReadUInt32();
            var chunkSize = rdr.ReadUInt32();
            var format = rdr.ReadUInt32();
            var subChunk1Id = rdr.ReadUInt32();
            var subChunk1Size = rdr.ReadUInt32();
            var audioFormat = rdr.ReadUInt16();
            var numChannels = rdr.ReadUInt16();
            var sampleRate = rdr.ReadUInt32();
            var byteRate = rdr.ReadUInt32();
            var blockAlign = rdr.ReadUInt16();
            var bitsPerSample = rdr.ReadUInt16();
            var subChunk2Id = rdr.ReadUInt32();
            var subChunk2Size = rdr.ReadUInt32();

            Console.WriteLine($"chunkId:       {chunkId:X8}");
            Console.WriteLine($"chunkSize:     {chunkSize}");
            Console.WriteLine($"format:        {format:X8}");
            Console.WriteLine($"subChunk1Id:   {subChunk1Id:X8}");
            Console.WriteLine($"subChunk1Size: {subChunk1Size}");
            Console.WriteLine($"audioFormat:   {audioFormat}");
            Console.WriteLine($"numChannels:   {numChannels}");
            Console.WriteLine($"sampleRate:    {sampleRate}");
            Console.WriteLine($"byteRate:      {byteRate}");
            Console.WriteLine($"blockAlign:    {blockAlign}");
            Console.WriteLine($"bitsPerSample: {bitsPerSample}");
            Console.WriteLine($"subChunk2Id:   {subChunk2Id:X8}");
            Console.WriteLine($"subChunk2Size: {subChunk2Size}");
        }

        static void WriteCodes(List<Code> codes)
        {
            var codeIdx = 1;

            foreach (var cde in codes)
            {
                if (!cde.Pulse.Last().IsHigh)
                {
                    cde.Pulse.RemoveAt(cde.Pulse.Count - 1);
                }

                if (cde.Pulse.Count == EXPECTED_LENGTH)
                {
                    Console.Write($"\n{codeIdx++:000} {cde.Pulse.Count:000} - ");

                    for (var pulseIndex = 2; pulseIndex < cde.Pulse.Count; pulseIndex += 2)
                    {
                        var highMW = cde.Pulse[pulseIndex].MS;
                        Console.Write(highMW > 0.5 ? "1" : "0");
                    }
                }
            }
        }

        const double SAMPLES_PER_MS = 2000.0;
        const double EXPECTED_LENGTH = 81;

        static void Main(string[] args)
        {
            var file = System.IO.File.OpenRead(@"S:\SDROutput\BlindControllerOut\swoff.bin");
            using (var rdr = new BinaryReader(file))
            {
                WriteHeader(rdr);
                var codes = new List<Code>();

                Code code = null;

                int sampleCount = 0;

                bool isLow = true;
                bool firstHit = false;

                double ms = 0;
                double toggleMS = 0;

                var value = file.ReadByte();
                while (value != -1)
                {
                    sampleCount++;

                    var level = Convert.ToByte(value);
                    ms = Math.Round(sampleCount / SAMPLES_PER_MS, 2);

                    if (level > 0 && isLow)
                    {
                        if (ms - toggleMS > 0.1)
                        {
                            if (!firstHit)
                            {
                                toggleMS = ms;
                                firstHit = true;
                            }

                            isLow = false;

                            if (ms - toggleMS < 5)
                            {
                                if (code != null)
                                {
                                    code.Pulse.Add(new Pulse() { MS = ms - toggleMS, IsHigh = false });
                                }

                                toggleMS = ms;
                            }
                            else
                            {
                                toggleMS = ms;
                                code = null;
                            }
                        }
                    }

                    if (level < 0xA0 && !isLow)
                    {
                        if (ms - toggleMS > 0.1)
                        {
                            isLow = true;

                            if (ms - toggleMS > 4)
                            {
                                code = new Code();
                                codes.Add(code);
                            }

                            if (code != null)
                            {
                                code.Pulse.Add(new Pulse() { MS = ms - toggleMS, IsHigh = true });
                            }

                            toggleMS = ms;
                        }
                    }

                    value = file.ReadByte();
                }

                WriteCodes(codes);
            }

            
            Console.ReadKey();
        }
    }
}
