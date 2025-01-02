using LagoVista.PickAndPlace.Interfaces;
using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace LagoVista.PickAndPlace.LumenSupport
{
    public enum FeederCommands
    {
        GetId = 1,
        Initialize = 2,
        GetVersion = 3,
        MoveFeedForward = 4,
        MoveFeedBackward = 5,
        MoveFeedStatus = 6,
        VendorOptions = 192,
        IdentifyFeeder = 193,
        ProgramFeederFloor = 194,
        UninitialziedFeedersRespond = 195,
    }

    public class CommandStructure
    {
        public bool IsUnicast { get; set; }
        public byte PayloadLength { get; set; }
    }

    public class PhotonResponse
    {
        public byte ToAddress { get; set; }
        public byte FromAddress { get; set; }
        public byte PacketId { get; set; }
        public byte Status { get; set; }
        public byte[] BinaryPayload { get; set; }
        public string TextPayload { get; set; }
    }
    public class PhotonCommand
    {
        public string GCode { get; set; }
        public byte PacketId { get; set; }
    }


    public class PhotonProtocolHandler : IPhotonProtocolHandler
    {
        public byte packetID = 0;

        private Dictionary<FeederCommands, CommandStructure> _feederCommands = new Dictionary<FeederCommands, CommandStructure>()
        {
            {FeederCommands.GetId, new CommandStructure() { PayloadLength = 1,} },
            {FeederCommands.Initialize, new CommandStructure() { PayloadLength = 1,} },
            {FeederCommands.GetVersion, new CommandStructure() { PayloadLength = 1,} },
            {FeederCommands.MoveFeedForward, new CommandStructure() { PayloadLength = 1,} },
            {FeederCommands.MoveFeedBackward, new CommandStructure() { PayloadLength = 1,} },
            {FeederCommands.MoveFeedStatus, new CommandStructure() { PayloadLength = 1,} },
            {FeederCommands.VendorOptions, new CommandStructure() { PayloadLength = 1,} },
            {FeederCommands.IdentifyFeeder, new CommandStructure() { PayloadLength = 13, IsUnicast = false} },
            {FeederCommands.ProgramFeederFloor, new CommandStructure() { PayloadLength = 14, IsUnicast = false} },
            {FeederCommands.UninitialziedFeedersRespond, new CommandStructure() { PayloadLength = 1, IsUnicast = false} },
        };


        public byte CalcCRC(List<byte> e)
        {
            ushort t = 0;
            for (int a = 0; a < e.Count(); a++)
            {
                t ^= (ushort)(e[a] << 8); // XOR with the byte shifted 8 bits left
                for (int n = 0; n < 8; n++)
                {
                    if ((t & 32768) != 0) // Check if the most significant bit is 1
                    {
                        t ^= 33664; // XOR with the polynomial value
                    }
                    t <<= 1; // Left shift by 1
                }
            }
            return (byte)((t >> 8) & 255); // Right shift by 8 and mask with 255 to get the final byte
        }

        public static int GetHexVal(char hex)
        {
            int val = (int)hex;
            return val - (val < 58 ? 48 : (val < 97 ? 55 : 87));
        }

        public static List<byte> GetByteArray(string hex)
        {
            if (hex.Length % 2 == 1)
                throw new Exception("The binary key cannot have an odd number of digits");

            byte[] arr = new byte[hex.Length >> 1];

            for (int i = 0; i < hex.Length >> 1; ++i)
            {
                arr[i] = (byte)((GetHexVal(hex[i << 1]) << 4) + (GetHexVal(hex[(i << 1) + 1])));
            }

            return new List<byte>(arr);
        }

        public PhotonResponse ParseResponse(string packet)
        {
            var response = new PhotonResponse();
            var buffer = GetByteArray(packet);
            response.ToAddress = buffer[0];
            response.FromAddress = buffer[1];
            response.PacketId = buffer[2];

            response.Status = buffer[5];

            response.BinaryPayload = buffer.Skip(6).ToArray();
            response.TextPayload = packet.Substring(12);

            var crc = buffer[4];

            buffer.RemoveAt(4);
            if(crc != CalcCRC(buffer))
            {
                throw new Exception("CRC Mismatch");
            }

            return response;
        }

        public PhotonCommand GenerateGCode(FeederCommands command, byte toAddress = 0, byte[] p = null)
        {
            var payload = p == null ? new List<byte>() : new List<byte>( p);
            payload.Insert(0, (byte)command);

            var cmdStructure = _feederCommands[command];
            if(cmdStructure.IsUnicast && toAddress == 0)
            {
                throw new ArgumentException($"Command {command} expects target");
            }

            if (cmdStructure.PayloadLength > 1 && (payload.Count == 0)) 
            {
                throw new ArgumentException($"Command {command} expects a payload");
            }

            if(cmdStructure.PayloadLength == 0 && (payload.Count == 0))
            {
                throw new ArgumentException($"Command {command} expects a payload.");
            }
            
            var buffer = new List<byte>
            {
                toAddress,
                0x00,
                packetID,
                cmdStructure.PayloadLength == 0 ? (byte)payload.Count : cmdStructure.PayloadLength
            };
            buffer.AddRange(payload);

            var crc = CalcCRC(buffer);
            buffer.Insert(4,crc);

            var bldr = new StringBuilder();
            foreach (var ch in buffer)
            {
                bldr.Append(ch.ToString("X2").ToUpper());
            }

            var cmd = new PhotonCommand()
            {
                GCode = $"M485 {bldr}",
                PacketId = packetID
            };
 
            packetID = (packetID < 255) ? (byte)(packetID + 1) : (byte)0;

            return cmd;        
        }
    }
}
