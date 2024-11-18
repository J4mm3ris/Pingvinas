import React from 'react'
import { FaAccessibleIcon } from "react-icons/fa";
import Link from 'next/link';

export default function Header() {
  return (
    <div className='flex justify-between item-center '>

        <div className="flex gap-9 ">
            <Link href="/">
                <img src="./logo.png" width="62" className='nbar'/>
                <img src="./text.png" width="200" className='nbar'/>
            </Link>
        </div>

        <div className='p-6'>
            <Link href="/settings" className='hover:text-blue-500 inline'>
                <FaAccessibleIcon className='text-6xl'/>
            </Link>
        </div>
    </div>

  )
}
