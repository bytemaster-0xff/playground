using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;

namespace DependencyScanner
{
    class Program
    {
        private class Reference
        {
            public AssemblyName Assembly { get; set; }
            public AssemblyName ReferencedAssembly { get; set; }
            public Version Version { get; set; }

        }

        private static List<Assembly> _assemblies = new List<Assembly>();
        private static List<Reference> _references = new List<Reference>();

        private static IEnumerable<IGrouping<string, Reference>> FindReferencesWithTheSameShortNameButDiffererntFullNames(List<Reference> references)
        {
            var refs = from assRef
                       in references
                       group assRef by assRef.ReferencedAssembly.FullName into assRefs
                       select new { assm = assRefs.Key, assRefs };

            var uniqueReferences = references.ToList().Select(reference => new { reference.ReferencedAssembly.FullName, reference.ReferencedAssembly.Version } ).Distinct().ToList();

            var conflicts = from reference in references
                   group reference by reference.ReferencedAssembly.Name
                       into referenceGroup
                   where uniqueReferences.Count() > 1
                   select referenceGroup;

            return conflicts;
        }

        private static void GetReferencesFromAssembly(Assembly assembly)
        {
            Console.WriteLine(assembly.FullName);
            foreach (var referencedAssembly in assembly.GetReferencedAssemblies())
            {
                Console.ForegroundColor = ConsoleColor.White;
                Console.WriteLine($"\t\t{referencedAssembly.FullName} - {referencedAssembly.Version}");
                _references.Add(new Reference
                {
                    Assembly = assembly.GetName(),
                    ReferencedAssembly = referencedAssembly,
                    Version = referencedAssembly.Version
                });
                Console.ResetColor();
            }
        }

        private static void GetReferencesFromAllAssemblies(List<Assembly> assemblies)
        {
            foreach (var assembly in assemblies)
            {
                GetReferencesFromAssembly(assembly);
                Console.WriteLine(" ");
            }
        }

        private static void GetAllAssemblies(string path)
        {
            if (!path.ToLower().EndsWith(".git"))
            {
                var dirs = System.IO.Directory.GetDirectories(path);
                foreach (var dir in dirs)
                {
                    GetAllAssemblies(dir);
                }

                var files = new List<FileInfo>();
                var directoryToSearch = new DirectoryInfo(path);
                files.AddRange(directoryToSearch.GetFiles("*.dll", SearchOption.AllDirectories));
                files.AddRange(directoryToSearch.GetFiles("*.exe", SearchOption.AllDirectories));


                foreach(var file in files)
                {
                    var assembly = Assembly.LoadFile(file.FullName);
                    if(!_assemblies.Where(asm=>asm.FullName == assembly.FullName).Any())
                    {
                        _assemblies.Add(assembly);
                    }
                }
            }
        }


        static int Main(string[] args)
        {
            if(args.Length != 1)
            {
                Console.WriteLine("Dependency Scanner - Usage: depscn [directory]");
            }


            GetAllAssemblies(args[0]);
            GetReferencesFromAllAssemblies(_assemblies);

            var groupsOfConflicts = FindReferencesWithTheSameShortNameButDiffererntFullNames(_references);
       
            foreach (var group in groupsOfConflicts)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.Out.WriteLine("Possible conflicts for {0}:", group.Key);
                Console.ForegroundColor = ConsoleColor.White;

                foreach (var reference in group)
                {
                    Console.Out.WriteLine($"\t\t{reference.Assembly.Name.PadRight(25)} references {reference.ReferencedAssembly.FullName} - {reference.ReferencedAssembly.Version}");
                }

                Console.ResetColor();
                Console.Out.WriteLine();
            }

            Console.WriteLine("Press any key to continue");
            Console.ReadKey();


            return groupsOfConflicts.Count();
        }
    }
}
